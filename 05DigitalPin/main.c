#include <stdio.h>
#include <string.h>
#include "shell.h"
#include "led.h"
#include "periph_conf.h"
#include "periph/gpio.h"
#include "xtimer.h"
#include "timex.h"
#include "crc16_ccitt.h"
#include "main.h"


#define TICK 3//<-- number of milli seconds per tick (--> use for delay())
#define CLOCK_HALF 5 // <-- number of ticks per half clock
#define CLOCK (2 * CLOCK_HALF) // <-- number of ticks per clock (1 data bit)
#define GET_CLASSIFIER_TICKS (TICK * CLOCK * 3)
#define MINIMUM_HIGH_LOW_DIFFERENCE 50 // <-- used in get_classifier()
#define HIGH 1
#define LOW 0
#define ENABLE_DEBUG  (1)// enable or disable debug
#include "debug.h"

int main(void) {
    send_pin = GPIO_PIN(PORT_F, 12);
    int isValid = (int) gpio_is_valid	(send_pin);
    if(isValid){
        gpio_init(send_pin, GPIO_OUT);
    }
    uint8_t* sendingData = (uint8_t*)01245675;

    send_data(sendingData,8);

    return 0;
}

void tick_start(void) {
    last_tick = xtimer_now_usec(); //time in micro seconds since start
}

void tick(uint8_t number_ticks) {
    uint32_t  now = xtimer_now_usec() ;
    uint32_t wait_till = last_tick + number_ticks * TICK * 1000;
    if (wait_till > now) {
        xtimer_usleep(wait_till - now);
    }
    last_tick = wait_till;
}

void man_one(void) {
    gpio_write(send_pin, LOW);
    tick(CLOCK_HALF);
    gpio_write(send_pin, HIGH);
    tick(CLOCK_HALF);
}

//manchester zero
void man_zero(void)
{
    gpio_write(send_pin, HIGH);
    tick(CLOCK_HALF);
    gpio_write(send_pin, LOW);
    tick(CLOCK_HALF);
}

uint8_t send_byte(uint8_t b, uint8_t c)
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
void send_sync(void)
{
    tick_start();
    for (int i = 0; i < 8; i++)
    {
        man_zero();
        man_one();
    }
}

//sending delimiter
void send_delimiter(void)
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
void send_data(const uint8_t *data, uint8_t data_len)
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
    DEBUG_PRINT("send_data(): Done");
}

