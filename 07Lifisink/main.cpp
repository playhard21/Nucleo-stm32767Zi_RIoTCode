#include <stdio.h>
#include <string.h>
/* RIOT APIs */
#include "arduino_board.h"
#include "arduino_pinmap.h"
#include "periph/adc.h"
#include "xtimer.h"
#include "timex.h"
#include "shell.h"
#include "led.h"
#include "periph_conf.h"
#include "periph/gpio.h"
/* header files */
#include "main.hpp"
#include "crc16_ccitt.hpp"
/* settings */
#define TICK 3//<-- number of milli seconds per tick (--> use for delay())
#define CLOCK_HALF 5 // <-- number of ticks per half clock
#define CLOCK (2 * CLOCK_HALF) // <-- number of ticks per clock (1 data bit)
#define GET_CLASSIFIER_TICKS (TICK * CLOCK * 3)
#define MINIMUM_HIGH_LOW_DIFFERENCE 50 // <-- used in get_classifier()
#define HIGH 1
#define LOW 0
#define ENABLE_DEBUG  (1)// enable or disable debug
#include "debug.h"
/* variables */
char debug_str_buf[256];
uint8_t debug_str_pos;

static const shell_command_t commands[] = {
        { NULL, NULL, NULL }
};

int main(void)
{
    char line_buf[SHELL_DEFAULT_BUFSIZE];
    LiFi(0);
    gpio_init_int(BTN0_PIN, GPIO_IN, GPIO_FALLING, gpio_falling, NULL);
    while(1){
        uint8_t ibuf[64];
        int ibuf_len = LiFi(0).receive(ibuf, sizeof(ibuf));
        if(ibuf_len >= 0){
            LED0_TOGGLE;
            LED1_TOGGLE;
            LED2_TOGGLE;
        }
    }
    shell_run(commands, line_buf, SHELL_DEFAULT_BUFSIZE);
    return 0;
}

LiFi::LiFi(int rLine)
{

    recv_line = rLine;
    adcPortinit = adc_init(recv_line);
    needs_synchronisation = 1;
}

// Initilize tick function
void LiFi::tick_start() {
    last_tick = xtimer_now_usec(); //time in micro seconds since start
}

// Delay execution by one tick
void LiFi::tick(uint8_t number_ticks) {
    uint32_t  now = xtimer_now_usec() ;
    uint32_t wait_till = last_tick + number_ticks * TICK * 1000;
    if (wait_till > now) {
        xtimer_usleep(wait_till - now);
    }
    last_tick = wait_till;
}

//Receiver Functions
//getclassifier
int LiFi::get_classifier()
{
    tick_start();
    int minimum = INT_MAX;
    int maximum = INT_MIN;
    for (int i = 0; i < GET_CLASSIFIER_TICKS; i++)
    {
        int val = adc_sample(recv_line, ADC_RES_10BIT);
        if (val < minimum)
            minimum = val;
        if (val > maximum)
            maximum = val;
        tick();
    }

    if (maximum - minimum < MINIMUM_HIGH_LOW_DIFFERENCE)
    {
        puts("\nget_classifier(): Aborted, difference too low. Max: ");
        return -1;
    }
    classifier = (maximum + minimum) / 2;
    puts("get_classifier(): ");
    return classifier;
}

int LiFi::get_level()
{
    if (adc_sample(recv_line, ADC_RES_10BIT) < classifier) {
        debug_str_buf[debug_str_pos++] = '_';
        return 0;
    }
    debug_str_buf[debug_str_pos++] = '-';
    return 1;
}

// clock syncronisation for correct data receiption
int LiFi::sync_clock()
{
    int count = 0;
    int old_value = get_level();
    for (int i = 0; i < 24 * CLOCK; i++)
    {
        int value = get_level();
        if (value == old_value)
        {
            count++;
        }
        else {
            if (count > (3 * CLOCK) / 4 )
            {
                tick(CLOCK_HALF);
                return 0;
            }
            old_value = value;
            count = 0;
        }
        tick();
    }
    DEBUG("LiFi::sync_clock(): Failed");
    return -1;
}

//manchester decoding getting bit value
int LiFi::get_bit()
{
    int first_half = 0;
    int second_half = 0;
    for (int i = 0; i < CLOCK_HALF; i++)
    {
        first_half += get_level();
        tick();
    }
    for (int i = 0; i < CLOCK_HALF; i++)
    {
        second_half += get_level();
        tick();
    }
    if ((first_half <= CLOCK_HALF / 2) && (second_half > CLOCK_HALF / 2))
    {
        return 1;
    }
    if ((first_half > CLOCK_HALF / 2) && (second_half <= CLOCK_HALF / 2))
    {
        return 0;
    }

    return -1;
}

//skipping count until it gets zero for getting delimiter
int LiFi::skip_till_zero()
{
    while (1)
    {
        switch (get_bit())
        {
            case 0:
                return 0;
            case -1:
                DEBUG("LiFi::skip_till_zero(): get_bit() failed");
                return -1;
            default:
                break;
        }
    }
}

// getting delimiter for finding the start and end of message
int LiFi::get_delimeter()
{
    if (skip_till_zero() == -1){
        DEBUG("LiFi::get_delimeter(): Failed on skip_till_zero()");
        return -1;
    }

    while (1)
    {
        restart_get_delimeter:
        for (int i = 0; i < 6; i++)
        {
            switch (get_bit())
            {
                case 0:
                    goto restart_get_delimeter;
                case -1:
                    DEBUG("LiFi::get_delimeter(): Failed on get_bit()");
                    return -1;
                default:
                    break;
            }
        }
        if (get_bit() == 0)
        {
            return 0;
        }
        return -1;
    }
}

// get byte of message
int LiFi::get_byte(uint8_t *dest)
{
    *dest = 0;
    for (int bitpos = 0; bitpos < 8; bitpos++)
    {
        if (ones_in_a_row >= 5)
        {
            switch (get_bit())
            {
                case -1:
                    DEBUG("LiFi::get_byte(): Failed on get_bit()");
                    return -1;
                case 0:
                    // Stuffed 0
                    ones_in_a_row = 0;
                    break;
                default:
                    // Seems to be a delimeter
                    if (get_bit() == 0){
                        return 1;
                    }
                    DEBUG("LiFi::get_byte(): Invalid data (corrupted delimeter?)");
                    return -1;
            }
        }
        switch (get_bit())
        {
            case 0:
                ones_in_a_row = 0;
                break;
            case 1:
                ones_in_a_row++;
                *dest |= 0x80 >> bitpos;
                //*dest |= 1 << bitpos;
                break;
            default:
                DEBUG("LiFi::get_byte(): Invalid data (corrupted delimeter?)");
                return -1;
        }
    }

    return 0;
}

//receiving message and storing in buffer
int LiFi::receive_frame(uint8_t *buf, uint8_t buf_size)
{
    uint8_t pos = 0;
    ones_in_a_row = 0;
    //Serial.print(buf_size);
    while (pos < buf_size)
    {
        switch (get_byte(&buf[pos]))
        {
            case -1:
                return -1;
            case 1: // End of message
                DEBUG("Got frame of size ");
                return pos;
            default:
                break;
        }
        //Serial.print(buf[pos]);
        pos++;
    }

    DEBUG("Buffer overflow");
    return -1;
}


// receive function for global calling
int LiFi::receive(uint8_t *buf, uint8_t buf_size)
{
    int result;
    debug_str_pos = 0;
    if (needs_synchronisation)
    {
        int classifier = get_classifier();
        if (classifier == -1) {
            debug_str_buf[debug_str_pos] = '\0';
            return -1;
        }
        if (sync_clock() == -1) {
            debug_str_buf[debug_str_pos] = '\0';
            return -1;
        }
        if (get_delimeter() == -1) {
            debug_str_buf[debug_str_pos] = '\0';
            return -1;
        }
    }
    result = receive_frame(buf, buf_size);
    needs_synchronisation = (result == -1) ? 1 : 0;

    debug_str_buf[debug_str_pos] = '\0';

    if (result == -1) {
        return -1;
    }

    if (result < 2) {
        DEBUG("receive(): Message too small to contain CRC-16");
        return -1;
    }
    int len = result - 2;
    uint16_t crc_exp = crc16_ccitt(buf, len);
    uint16_t crc_got = ((uint16_t)buf[len]) << 8 | ((uint16_t)buf[len + 1]);
    if (crc_exp != crc_got) {
        DEBUG("receive(): CRC didn't match! (Expected = 0x");
        DEBUG(", but received = 0x");
        return -1;
    }

    DEBUG("receive(): Successfully received frame with PDU size ");

#ifdef ENABLE_DEBUG
    DEBUG("PDU: ");
    for (int i = 0; i < len - 1; i++) {
        DEBUG("0x");
        DEBUG(", ");
    }
    DEBUG("0x");
#endif

    return len;
}