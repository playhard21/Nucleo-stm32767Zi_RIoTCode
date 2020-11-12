//
// Created by aki on 12.11.20.
//

#ifndef _MAIN_H
#define _MAIN_H

#pragma once
#include "periph/adc.h"
#include <stdint.h>

//variables
gpio_t send_pin; // <-- this is the digital pin number for sending
uint32_t  last_tick; // <-- timestamp of the last tim
mutex_t mutex;
uint8_t obuf[64];
uint8_t obuf_len;

//Global variables
char tx_stack1[THREAD_STACKSIZE_MAIN];

void setup(void);
int send_data(const uint8_t *data, uint8_t data_len);
void *tx_loop(void *arg);

class LiFi {
    // pravite members and functions
private:
    //transmitter members and functions
    int send_pin; // <-- this is the digital pin number for sending
    uint32_t  last_tick; // <-- timestamp of the last tim
    void tick(uint8_t number_ticks = 1); // <-- wait for that number of ticks
    void tick_start(); // <-- Initialize tick()
    void man_one();//<-- sending manchester one
    void man_zero();//<--sending macherster zero
    uint8_t send_byte(uint8_t b, uint8_t ones_in_a_row);//<-- sending one uint8_t of data
    void send_sync();
    void send_delimiter(void);
    //public functions
public:
    //sendingFunctions
    LiFi(gpio_t sPin);
    void send_data(const uint8_t *data, uint8_t data_len);
};


#endif
