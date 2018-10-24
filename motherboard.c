#include <stdio.h>
#include <malloc.h>

#include "motherboard.h"

void mb_run(struct motherboard *mb)
{
    int cycles = 0;
    struct iapx88 *cpu = mb->cpu;
    while (1) {
	int eu_cycles = iapx88_step(mb->cpu);
	int biu_cycles = 0;
	printf("Ran %d cycles\n", eu_cycles);
	if (eu_cycles < 0) {
	    return;
	}
	cycles += cpu_cycles;
	if (cpu->control_bus_state == NONE) {
	    /* Bus is unused, prefetch until biu catches up with eu */
	    
	    
	} else {
	    /* EU is waiting for something */
	    switch (cpu->control_bus_state) {
	    case MEMREAD:
	    case FETCH:
		printf("Reading memory at %X\n", cpu->address_pins);
		cpu->data_pins = mb->ram[cpu->address_pins];
		break;
	    case MEMWRITE:
		if (cpu->address_pins < 0xC0000) {
		    mb->ram[cpu->address_pins] = cpu->data_pins;
		}
		break;
	    }
	}
    }
}

struct motherboard *mb_create()
{
    struct motherboard *mb = (struct motherboard *)malloc(sizeof(struct motherboard));
    mb->cpu = iapx88_create();
    return mb;
}

void mb_powerup(struct motherboard *mb)
{
    iapx88_reset(mb->cpu);
}

int mb_load_bios_rom(struct motherboard *mb, const char *filename)
{
    FILE *f = fopen(filename, "rb");
    int r = fread(mb->ram + 0xFE000, 1, 0x2000, f);
    fclose(f);
    return r;
}
