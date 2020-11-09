/*
 * Copyright (C) 2020 OVGU
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     module_LiFi
 * @{
 *
 * @file
 * @brief       LiFi implementation
 *
 * @author      AravindKarri <aravind.karri@st.ovgu.de>
 *
 * @}
 */
/* MY APIs */
#include "crc16_ccitt.h"
#include "LiFi.h"
/* Debug */
#include "debug.h"
/* RIOT APIs */
#include "thread.h"
#include "xtimer.h"
#include "timex.h"
#include "periph/adc.h"
#include "periph_conf.h"
#include "periph/gpio.h"
/* Implementation of the module */
#define APP_NAME	"LiFi"
#define TICK 3//<-- number of milli seconds per tick (--> use for delay())
#define CLOCK_HALF 5 // <-- number of ticks per half clock
#define CLOCK (2 * CLOCK_HALF) // <-- number of ticks per clock (1 data bit)
#define GET_CLASSIFIER_TICKS (TICK * CLOCK * 3)
#define MINIMUM_HIGH_LOW_DIFFERENCE 50 // <-- used in get_classifier()

char debug_str_buf[256];
uint8_t debug_str_pos;


//Transmitter functions
//declaration of transmitter pins
LiFi::LiFi(int sPin, adc_t rPin)
{
    send_pin = sPin;
    gpio_init(send_pin,GPIO_OUT);
    recv_pin = rPin;
    adc_init(recv_pin);
    // classifier = 0;
    //    ones_in_a_row = 0;
    needs_synchronisation = 1;
}

// Initilize tick function
void LiFi::tick_start() {
    last_tick = micros();
}

// Delay execution by one tick
void LiFi::tick(uint8_t number_ticks) {
    unsigned long now = micros();
    unsigned long wait_till = last_tick + number_ticks * TICK * 1000;
    if (wait_till > now) {
        delayMicroseconds(wait_till - now);
    }
    last_tick = wait_till;
}

// manchester one
void LiFi::man_one()
{
    gpio_write(send_pin, LOW);
    tick(CLOCK_HALF);
    gpio_write(send_pin, HIGH);
    tick(CLOCK_HALF);
}

//manchester zero
void LiFi::man_zero()
{
    gpio_write(send_pin, HIGH);
    tick(CLOCK_HALF);
    gpio_write(send_pin, LOW);
    tick(CLOCK_HALF);
}

// send uint8_t
uint8_t LiFi::send_byte(uint8_t b, uint8_t c)
{
    for (int i = 0; i < 8; i++)
    {
        bool current_bit = ((b << i) & 0x80) ;
        if (current_bit)
        {
            c++;
            man_one();
            if (c == 5)
            {
                man_zero();
                // Serial.println("encountered 5 ones");
                c = 0;
            }
        }
        else
        {
            man_zero();
            c = 0;
        }
    }

    return c;
}

//sending sync uint8_t
void LiFi::send_sync()
{
    tick_start();
    for (int i = 0; i < 8; i++)
    {
        man_zero();
        man_one();
    }
}

//sending delimiter
void LiFi::send_delimiter(void)
{
    static const uint8_t delim = 0x7e;
    for (int i = 0; i < 8; i++)
    {
        bool c_b = ((delim << i) & 0x80) ;
        if (c_b)
            man_one();
        else
            man_zero();
    }
}

//sending data
void LiFi::send_data(const uint8_t *data, uint8_t data_len)
{
    send_sync();
    send_delimiter();
    uint8_t ones_in_a_row = 0;
    for (uint8_t i = 0; i < data_len; i++)
    {
        ones_in_a_row = send_byte(data[i], ones_in_a_row);
    }
    uint16_t crc = crc16_ccitt(data, data_len);
    DEBUGLN(crc, HEX);
    ones_in_a_row = send_byte((uint8_t)(crc >> 8), ones_in_a_row);
    ones_in_a_row = send_byte((uint8_t)crc, ones_in_a_row);
    send_delimiter();
    DEBUGLN("send_data(): Done");
}

// receiver functions
//getclassifier
int LiFi::get_classifier()
{
    tick_start();
    int minimum = INT_MAX;
    int maximum = INT_MIN;
    for (int i = 0; i < GET_CLASSIFIER_TICKS; i++)
    {
        int val = adc_sample(recv_pin, ADC_RES_10BIT);
        if (val < minimum)
            minimum = val;
        if (val > maximum)
            maximum = val;
        tick();
    }

    if (maximum - minimum < MINIMUM_HIGH_LOW_DIFFERENCE)
    {
        DEBUG("get_classifier(): Aborted, difference too low. Max: ");
        DEBUG(maximum);
        DEBUG(" Min: ");
        DEBUGLN(minimum);
        return -1;
    }
    classifier = (maximum + minimum) / 2;
    DEBUG("get_classifier(): ");
    DEBUGLN(classifier);
    return classifier;
}

// getting analog level for logic 1 and zero callibration
int LiFi::get_level()
{
    if (adc_sample(recv_pin, ADC_RES_10BIT) < classifier) {
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
    DEBUGLN("LiFi::sync_clock(): Failed");
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
                DEBUGLN("LiFi::skip_till_zero(): get_bit() failed");
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
        DEBUGLN("LiFi::get_delimeter(): Failed on skip_till_zero()");
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
                    DEBUGLN("LiFi::get_delimeter(): Failed on get_bit()");
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
                    DEBUGLN("LiFi::get_byte(): Failed on get_bit()");
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
                    DEBUGLN("LiFi::get_byte(): Invalid data (corrupted delimeter?)");
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
                DEBUGLN("LiFi::get_byte(): Invalid data (corrupted delimeter?)");
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
                DEBUGLN((int)pos);
                return pos;
            default:
                break;
        }
        //Serial.print(buf[pos]);
        pos++;
    }

    DEBUGLN("Buffer overflow");
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
            DEBUGLN(debug_str_buf);
            return -1;
        }
        if (sync_clock() == -1) {
            debug_str_buf[debug_str_pos] = '\0';
            DEBUGLN(debug_str_buf);
            return -1;
        }
        if (get_delimeter() == -1) {
            debug_str_buf[debug_str_pos] = '\0';
            DEBUGLN(debug_str_buf);
            return -1;
        }
    }
    result = receive_frame(buf, buf_size);
    needs_synchronisation = (result == -1) ? 1 : 0;

    debug_str_buf[debug_str_pos] = '\0';
    DEBUGLN(debug_str_buf);

    if (result == -1) {
        return -1;
    }

    if (result < 2) {
        DEBUGLN("receive(): Message too small to contain CRC-16");
        return -1;
    }
    int len = result - 2;
    uint16_t crc_exp = crc16_ccitt(buf, len);
    uint16_t crc_got = ((uint16_t)buf[len]) << 8 | ((uint16_t)buf[len + 1]);
    if (crc_exp != crc_got) {
        DEBUG("receive(): CRC didn't match! (Expected = 0x");
        DEBUG(crc_exp, HEX);
        DEBUG(", but received = 0x");
        DEBUGLN(crc_got, HEX);
        return -1;
    }

    DEBUG("receive(): Successfully received frame with PDU size ");
    DEBUGLN(len);

#ifdef ENABLE_DEBUG
    DEBUG("PDU: ");
    for (int i = 0; i < len - 1; i++) {
        DEBUG("0x");
        DEBUG(buf[i], HEX);
        DEBUG(", ");
    }
    DEBUG("0x");
    DEBUGLN(buf[len-1], HEX);
#endif

    return len;
}
