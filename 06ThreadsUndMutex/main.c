
#include <stdio.h>
#include <string.h>
#include "periph/gpio.h"
#include "shell.h"
#include "periph_conf.h"
#include "xtimer.h"
#include "timex.h"
#include "crc16_ccitt.h"
#include "main.h"

//Global variables
char tx_stack1[THREAD_STACKSIZE_MAIN];



int main(void)
{
    send_pin = GPIO_PIN(PORT_F, 12);
    int isValid = (int) gpio_is_valid	(send_pin);
    if(isValid){
        gpio_init(send_pin, GPIO_OUT);
    }

    uint8_t* SendingData = (uint8_t*)01245675;

    send_data(SendingData,8); //move memory to buffer
    setup();

    return 0;
};

void setup(void)
{

    thread_create(tx_stack1,
                  sizeof(tx_stack1),
                  THREAD_PRIORITY_MAIN - 1,
                  THREAD_CREATE_STACKTEST,
                  tx_loop,
                  NULL,
                  "tx1");

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
    for(uint8_t i=0 ; i < 30 ; i++ ) {
        mutex_lock(&mutex);
        uint8_t len = obuf_len;
        mutex_unlock(&mutex);
        if (len > 0) {
            printf("PLiFi::tx_loop(): TX starting");
            sendData(obuf, len);
            mutex_lock(&mutex);
            obuf_len = 0;
            mutex_unlock(&mutex);
        }
    }
    return NULL;
};

//sending functions
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
void sendData(const uint8_t *data, uint8_t data_len)
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
    printf("send_data(): Done");
}
