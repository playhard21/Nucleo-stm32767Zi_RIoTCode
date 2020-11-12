//
// Created by aki on 10.11.20.
//

#ifndef TRYNUCLEOCODE_MAIN_H
#define TRYNUCLEOCODE_MAIN_H
#pragma once
#include "xtimer.h"
#include "periph/adc.h"
#include "periph_conf.h"
#include <stdint.h>

//transmitter members and functions
gpio_t send_pin; // <-- this is the digital pin number for sending
uint32_t  last_tick; // <-- timestamp of the last tim
void tick(uint8_t); // <-- wait for that number of ticks
void tick_start(void); // <-- Initialize tick()
void man_one(void);//<-- sending manchester one
void man_zero(void);//<--sending macherster zero
uint8_t send_byte(uint8_t b, uint8_t ones_in_a_row);//<-- sending one uint8_t of data
void send_sync(void);
void send_delimiter(void);
//receiver members and functions
adc_t recv_pin; // <-- this is the analog pin number for receiving
int classifier; // <-- if analogRead(recv_pin) is less than this, treat
//     it as LOW, otherwise HIGH
int ones_in_a_row; // <-- Number of '1' bits received in a row in
int needs_synchronisation;
int get_classifier(void);
int get_level(void);
int sync_clock(void);
int get_bit(void);
int skip_till_zero(void);
int get_delimeter(void);
int get_byte(uint8_t *dest);
int receive_frame(uint8_t *buf, uint8_t buf_size);

//transmitter fuctions
void send_data(const uint8_t *data, uint8_t data_len);
//receiver functions
//  LiFi(int rPin);
int receive(uint8_t *buf, uint8_t buf_size);

#endif
