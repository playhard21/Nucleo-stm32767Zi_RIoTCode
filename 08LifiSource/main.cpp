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

#define TICK 3//<-- number of milli seconds per tick (--> use for delay())
#define CLOCK_HALF 5 // <-- number of ticks per half clock
#define CLOCK (2 * CLOCK_HALF) // <-- number of ticks per clock (1 data bit)
#define GET_CLASSIFIER_TICKS (TICK * CLOCK * 3)
#define MINIMUM_HIGH_LOW_DIFFERENCE 50 // <-- used in get_classifier()
#define HIGH 1
#define LOW 0
#define ENABLE_DEBUG  (1)// enable or disable debug
#include "debug.h"



static const shell_command_t commands[] = {
        { NULL, NULL, NULL }
};



int main(void)
{
    char line_buf[SHELL_DEFAULT_BUFSIZE];
    send_pin = GPIO_PIN(PORT_F, 12);
    //send_pin = GPIO_PIN(PORT_A, 8);
    uint8_t* SendingData = (uint8_t*)124567598;
    LiFi(send_pin).send_data(SendingData,1); //move memory to buffer
    //setup();
    shell_run(commands, line_buf, SHELL_DEFAULT_BUFSIZE);
    return 0;
}

void setup(void)
{

    thread_create(tx_stack1,
                  sizeof(tx_stack1),
                  THREAD_PRIORITY_MAIN - 1,
                  THREAD_CREATE_STACKTEST,
                  tx_loop,
                  NULL,
                  "tx");

}

int send_data(const uint8_t *data, uint8_t data_len)
{ //this function copys data to output buffer
    if (data_len > sizeof(obuf)) {
        printf("send_data(): Called with too huge message");
        return -1;
    }

    mutex_lock(&mutex);
    if (obuf_len != 0) {
        mutex_unlock(&mutex);
        printf("send_data(): TX is still ongoing, refusing to send next frame");
        return -1;
    }

    memcpy(obuf, data, data_len);
    obuf_len = data_len;
    mutex_unlock(&mutex);
    return 0;
};

void *tx_loop(void *arg)
{
    (void)arg;
    while(1) {
        mutex_lock(&mutex);
        uint8_t len = obuf_len;
        mutex_unlock(&mutex);
        if (len > 0) {
            printf("PLiFi::tx_loop(): TX starting");
            LiFi(send_pin).send_data(obuf, len);
            mutex_lock(&mutex);
            obuf_len = 0;
            mutex_unlock(&mutex);
        }
    }
    return NULL;
};


LiFi::LiFi(gpio_t sPin)
{
    send_pin = sPin;
    gpio_init(send_pin, GPIO_OUT);
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
    ones_in_a_row = send_byte((uint8_t)(crc >> 8), ones_in_a_row);
    ones_in_a_row = send_byte((uint8_t)crc, ones_in_a_row);
    send_delimiter();
    DEBUG("send_data(): Done");
}