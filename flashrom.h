#pragma once
#include <stdint.h>

int flashrom_read(int32_t /* r4 */ start_pos, uint8_t* /* r5 */ dest, int32_t /* r6 */ size);
