//
// Created by aki on 12.11.20.
//

#ifndef _MAIN_H
#define _MAIN_H

#pragma once
#include "periph/adc.h"
#include <stdint.h>

class LiFi {
    // pravite members and functions
private:
    //transmitter members and functions
    uint32_t  last_tick; // <-- timestamp of the last tim
    void tick(uint8_t number_ticks = 1); // <-- wait for that number of ticks
    void tick_start(); // <-- Initialize tick()
    //receiver members and functions
    adc_t recv_line; // <-- this is the analog pin number for receiving
    int classifier; // <-- if analogRead(recv_pin) is less than this, treat
    //     it as LOW, otherwise HIGH
    int ones_in_a_row; // <-- Number of '1' bits received in a row in
    // get_uint8_t()
    int needs_synchronisation;
    int adcPortinit;
    int get_classifier();
    int get_level();
    int sync_clock();
    int get_bit();
    int skip_till_zero();
    int get_delimeter();
    int get_byte(uint8_t *dest);
    int receive_frame(uint8_t *buf, uint8_t buf_size);
    //public functions
public:
    //receiver functions
    LiFi(int rLine);
    int receive(uint8_t *buf, uint8_t buf_size);
};


#endif
