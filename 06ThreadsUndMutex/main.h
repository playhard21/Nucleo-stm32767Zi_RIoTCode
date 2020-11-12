//
// Created by aki on 11.11.20.
//

#ifndef TRYNUCLEOCODE_MAIN_H
#define TRYNUCLEOCODE_MAIN_H

#pragma once
#include "mutex.h"
#include "xtimer.h"
#include "periph/adc.h"
#include "periph_conf.h"
#include <stdint.h>

//macros for use
#define TICK 3//<-- number of milli seconds per tick (--> use for delay())
#define CLOCK_HALF 5 // <-- number of ticks per half clock
#define CLOCK (2 * CLOCK_HALF) // <-- number of ticks per clock (1 data bit)
#define GET_CLASSIFIER_TICKS (TICK * CLOCK * 3)
#define MINIMUM_HIGH_LOW_DIFFERENCE 50 // <-- used in get_classifier()
#define HIGH 1
#define LOW 0

//transmitter members and functions

void tick(uint8_t); // <-- wait for that number of ticks
void tick_start(void); // <-- Initialize tick()
void man_one(void);//<-- sending manchester one
void man_zero(void);//<--sending macherster zero
uint8_t send_byte(uint8_t b, uint8_t ones_in_a_row);//<-- sending one uint8_t of data
void send_sync(void);
void send_delimiter(void);

//variables
gpio_t send_pin; // <-- this is the digital pin number for sending
uint32_t  last_tick; // <-- timestamp of the last tim
mutex_t mutex;
uint8_t obuf[64];
uint8_t obuf_len;


//transmitter fuctions
extern void sendData(const uint8_t *data, uint8_t data_len);
int send_data(const uint8_t *data, uint8_t data_len);
void *tx_loop(void *arg);
void setup(void);

#endif
