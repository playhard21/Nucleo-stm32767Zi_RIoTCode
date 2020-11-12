//
// Created by aki on 10.11.20.
//

#ifndef TRYNUCLEOCODE_CRC16_CCITT_H
#define TRYNUCLEOCODE_CRC16_CCITT_H
#pragma once
#include <stdint.h>
#include <stddef.h>

uint16_t crc16_ccitt(const uint8_t *data, uint8_t data_len);

#endif //TRYNUCLEOCODE_CRC16_CCITT_H
