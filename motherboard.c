#include <stdio.h>
#if defined(__MACH__)
#include <stdlib.h>
#else
#include <malloc.h>
#endif

#include "motherboard.h"

int access_memory(struct motherboard *mb, struct iapx88 *cpu)
{
    switch (cpu->control_bus_state) {
    case BUS_MEMREAD:
    case BUS_FETCH:
	printf("Reading memory at %X\n", cpu->address_pins);
	cpu->data_pins = mb->ram[cpu->address_pins];
	break;
    case BUS_MEMWRITE:
	if (cpu->address_pins < 0xC0000) {
	    mb->ram[cpu->address_pins] = cpu->data_pins;
	}
	break;
    default:
	printf("Unhandled bus state\n");
    }
    return 0;
}

void mb_run(struct motherboard *mb)
{
    int cycles = 0;
    struct iapx88 *cpu = mb->cpu;
    while (1) {
	int eu_cycles = iapx88_step(mb->cpu);
	int biu_cycles = 0;
	printf("EU ran for %d cycles\n", eu_cycles);
	if (eu_cycles < 0) {
	    return;
	}

	cycles += eu_cycles;
        if (!cpu->prefetch_forbidden) {
            while (biu_cycles < eu_cycles) {
                biu_cycles += biu_request_prefetch(cpu, cycles);
                if (cpu->bus_state == BUS_T3) {
                    biu_cycles += access_memory(mb, cpu);
                    biu_cycles += biu_handle_prefetch(cpu);
                }
            }
            cpu->prefetch_forbidden = 0;
        }

	switch (cpu->return_reason) {
	case WAIT_BIU:
	    printf("EU is waiting for BIU\n");
	    biu_cycles += biu_make_request(cpu);
	    biu_cycles += access_memory(mb, cpu);
	    biu_cycles += biu_handle_response(cpu);
	    printf("BIU ran for %d cycles\n", biu_cycles);
	    break;
	case WAIT_INTERRUPTIBLE:
	    printf("CPU is waiting for a possible interrupt\n");
	    break;
	case NO_REASON:
	    printf("CPU is waiting for no reason!\n");
	    break;
	}
	if (eu_cycles < biu_cycles) {
	    // eu needs to idle for a bit
	}
	printf("Continue? ");
	int cont = getc(stdin);
	if (cont == 'n') break;
	printf("\n");
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
