#include <string.h>

#include "flashrom.h"

char* const ROM_SYSTEM_ID = (char*)0x1A056;
char* const ROM_ICON_START = (char*)0x1A480;

char* const RAM_BASE_ADDRESS = (char*)0x8C00000;
char* const RAM_SYSTEM_ID = (char*)0x8C00068;

int sysinfo_init() {
    // The BIOS makes sure to call the entry point of FLASHROM.
    // We directly call the target function.
    // This may break games if they patch the entry point of FLASHROM.
    flashrom_read(ROM_SYSTEM_ID, RAM_SYSTEM_ID, 8);
    flashrom_read((void*)0x1A000, RAM_BASE_ADDRESS + 0x70, 5);
    memset(RAM_BASE_ADDRESS + 0x75, 0, 10);
    return 0;
}

/* void */ int sysinfo_invalid() {
    while (1) {}
}

int sysinfo_icon(unsigned int /* r4 */ icon_number, char* /* r5 */ dest) {
    if (icon_number >= 10) return -1; // Out of range

    const int ICON_SIZE = 0x2C0;
    unsigned int offset = icon_number * ICON_SIZE;
    return flashrom_read(ROM_ICON_START + offset, dest, ICON_SIZE);
}

/* unsigned long long* */ int sysinfo_id() {
    return (int)RAM_SYSTEM_ID;
}

int sysinfo(unsigned char /* r7 */ syscall, int /* r4 */ icon_number, char* /* r5 */ dest) {
    if (syscall >= 4) return -1; // No-op

    switch (syscall) {
        case 0: return sysinfo_init();
        case 1: return sysinfo_invalid(); // Not a valid syscall
        case 2: return sysinfo_icon(icon_number, dest);
        case 3: return sysinfo_id();
    }
}
