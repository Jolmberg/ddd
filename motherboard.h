#ifndef _MOTHERBOARD_H_
#define _MOTHERBOARD_H_

#include <stdio.h>
#include <stdint.h>
#include <pthread.h>

#include "8088.h"

struct motherboard {
    struct iapx88 *cpu;
    uint8_t ram[0x100000];
    int debug;
    pthread_mutex_t mutex;
    pthread_cond_t condition;
    int step;
};

struct motherboard *mb_create(void);
void *mb_run(void *mbarg);
int mb_load_bios_rom(struct motherboard *mb, const char *filename);
void mb_powerup(struct motherboard *mb);
uint8_t mb_memory_peek_absolute(struct motherboard *mb, uint32_t address);
uint8_t mb_memory_peek(struct motherboard *mb, uint16_t segment, uint16_t offset);

#endif
