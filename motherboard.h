#ifndef _MOTHERBOARD_H_
#define _MOTHERBOARD_H_

#include <stdio.h>
#include <stdint.h>

#include "8088.h"

struct motherboard {
    struct iapx88 *cpu;
    uint8_t ram[0x100000];
};

struct motherboard *mb_create(void);
void mb_run(struct motherboard *mb);
int mb_load_bios_rom(struct motherboard *mb, const char *filename);
void mb_powerup(struct motherboard *mb);

#endif
