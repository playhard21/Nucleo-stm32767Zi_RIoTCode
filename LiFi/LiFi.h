/**
 * Author:	aki
 * Created:	2020
 **/

#ifndef LIFI_H
#define LIFI_H
#pragma once
/* RIOT APIs */
#include <stdint.h>
#include "periph/adc.h"
#include "periph_conf.h"
#include "periph/gpio.h"


class LiFi {
    // pravite members and functions
private:
    //transmitter members and functions
    int send_pin; // <-- this is the digital pin number for sending
    unsigned long last_tick; // <-- timestamp of the last tim
    void tick(uint8_t number_ticks = 1); // <-- wait for that number of ticks
    void tick_start(); // <-- Initialize tick()
    void man_one();//<-- sending manchester one
    void man_zero();//<--sending macherster zero
    uint8_t send_byte(uint8_t b, uint8_t ones_in_a_row);//<-- sending one uint8_t of data
    void send_sync();
    void send_delimiter(void);
    //receiver members and functions
    adc_t recv_pin; // <-- this is the analog pin number for receiving
    int classifier; // <-- if analogRead(recv_pin) is less than this, treat
    //     it as LOW, otherwise HIGH
    int ones_in_a_row; // <-- Number of '1' bits received in a row in
    // get_uint8_t()
    int needs_synchronisation;
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
    //transmitter fuctions
    LiFi(int sPin, adc_t rPin);
    void send_data(const uint8_t *data, uint8_t data_len);
    //receiver functions
    //  LiFi(int rPin);
    int receive(uint8_t *buf, uint8_t buf_size);
};
#endif
