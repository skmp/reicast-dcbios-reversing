#include <stdint.h>
#include <string.h>

#include "flashrom.h"

char* const RAM_BASE_ADDRESS = (char*)0x8C000000;

struct rominfo {
    uint16_t start_offset; // start of struct
    uint16_t size;         // start + 2
};

#define RAM_PARTITION_INFO 0x8C003FA0

struct rominfo* const partition_info = (struct rominfo*)(RAM_PARTITION_INFO);

int flashrom_info(int32_t /* r4 */ partition, int32_t* /* r5 */ dest) {
    if (partition < 0 || partition >= 5) // Out of bounds
        return -1;

    *dest = partition_info[partition].start_offset << 8;
    *(dest+1) = partition_info[partition].size << 8;
    return 0;
}

uint8_t* const ROM_FLASH_WRITE_LOCK = (uint8_t*)0x8C00002D;
uint8_t* const ROM_FLASH_READ_LOCK = (uint8_t*)0x8C00002F;
const uint8_t* const ROM_FLASH_BASE_ADDRESS = (uint8_t*)0xA0200000;

int try_lock_mutex() {
    *ROM_FLASH_READ_LOCK = 1;
    if (*ROM_FLASH_WRITE_LOCK == 0) return 0;
    *ROM_FLASH_READ_LOCK = 0;
    return -1;
}

int unlock_mutex() {
    *ROM_FLASH_READ_LOCK = 0;
    return 0;
}

int flashrom_read(int32_t /* r4 */ start_pos, uint8_t* /* r5 */ dest, int32_t /* r6 */ size) {
    if (try_lock_mutex() != 0) return -1;
    // As a fun corner case: it's legal to pass in a negative value for size, which does nothing.
    if (size > 0) memcpy(dest, ROM_FLASH_BASE_ADDRESS + start_pos, size);
    unlock_mutex();
    return 0;
}

uint32_t STATUS = 0xFFFFFFFF; // processor status register

#define SR_IMASK 0xF0

int sub_8C003D44(uint8_t* /* r4 */ rom_ptr, uint8_t /* r5 */ source_byte) {


int write_byte_to_rom(uint8_t* /* r4 */ source, uint8_t* /* r5 */ rom_ptr) {
    // Select AMD ID part 1.
    // Select AMD ID part 2.
    // Program byte.
    *rom_ptr = *source;
    return sub_8C003D44(rom_ptr, *source);
}

int flashrom_write(int32_t /* r4 */ start_pos, uint8_t* /* r5 */ source, int32_t /* r6 */ size) {
    if (try_lock_mutex() != 0) return -1;
    int interrupts = STATUS & SR_IMASK;
    STATUS = (STATUS & ~SR_IMASK) | 0xF0; // mask interrupts
    uint8_t* const rom_ptr = ROM_FLASH_BASE_ADDRESS + start_pos;

    for (int bytes_written = 0; bytes_written < size; bytes_written++) {
        uint8_t source_byte = *source;
        uint8_t dest_byte = *rom_ptr;
        dest_byte ^= source_byte;
        if (!(dest_byte & source_byte)) break;


    unlock_mutex();
    return size;
}

int flashrom_delete(int /* r4 */ partition) {
    return -1;
}

int flashrom(uint8_t /* r7 */ syscall, int /* r4 */ arg1, int /* r5 */ arg2, int /* r6 */ arg3) {
    if (syscall >= 4) return -1;
    switch (syscall) {
        case 0: return flashrom_info(arg1, (void*)arg2);
        case 1: return flashrom_read(arg1, (uint8_t*)arg2, arg3);
        case 2: return flashrom_write(arg1, (uint8_t*)arg2, arg3);
        case 3: return flashrom_delete(arg1);
    }
}
