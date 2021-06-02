#pragma once

#include <stdint.h>

using crc = uint8_t;

crc crcSlow(uint8_t* const message, int nBytes);