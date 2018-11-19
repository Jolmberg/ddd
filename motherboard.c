#include <stdio.h>
#include <pthread.h>
#if defined(__MACH__)
#include <stdlib.h>
#else
#include <malloc.h>
#endif

#include "motherboard.h"
#include "8088.h"

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

uint8_t mb_memory_peek_absolute(struct motherboard *mb, uint32_t address)
{
    // Do smart stuff here if needed
    return mb->ram[address];
}

uint8_t mb_memory_peek(struct motherboard *mb, uint16_t segment, uint16_t offset)
{
    return mb_memory_peek_absolute(mb, EA(segment, offset));
}

void *mb_run(void *mbarg)
{
    struct motherboard *mb = mbarg;
    int cycles = 0;
    struct iapx88 *cpu = mb->cpu;
    while (1) {
        if (mb->debug) {
		pthread_mutex_lock(&mb->mutex);
		pthread_cond_wait(&mb->condition, &mb->mutex);
		pthread_mutex_unlock(&mb->mutex);
        }

	int eu_cycles = (*cpu->next_step)(mb->cpu); //iapx88_step(mb->cpu);
	int biu_cycles = 0;
	printf("EU ran for %d cycles\n", eu_cycles);
	if (eu_cycles < 0) {
	    return NULL;
	}

	cycles += eu_cycles;
        if (!cpu->prefetch_forbidden) {
            while (biu_cycles < eu_cycles) {
                biu_cycles += biu_request_prefetch(cpu, eu_cycles - biu_cycles);
                if (cpu->bus_state == BUS_T3) {
                    biu_cycles += access_memory(mb, cpu);
                    biu_cycles += biu_handle_prefetch(cpu);
                }
            }
            printf("BIU prefetched for %d cycles\n", biu_cycles);
        } else {
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
	    mb->step++;
	    /* if (mb->debug) { */
	    /*     pthread_mutex_lock(&mb->mutex); */
	    /*     pthread_cond_wait(&mb->condition, &mb->mutex); */
	    /*     pthread_mutex_unlock(&mb->mutex); */
	    /* } */
	    break;
	case NO_REASON:
	    printf("CPU is waiting for no reason!\n");
	    break;
	}
	if (eu_cycles < biu_cycles) {
	    // eu needs to idle for a bit
	}
	/* printf("Continue? "); */
	/* int cont = getc(stdin); */
	/* if (cont == 'n') break; */
	/* printf("\n"); */
    }
    return NULL;
}

struct motherboard *mb_create()
{
    struct motherboard *mb = (struct motherboard *)malloc(sizeof(struct motherboard));
    mb->cpu = iapx88_create();
    mb->step = 0;
    pthread_mutex_init(&mb->mutex, NULL); //PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_init(&mb->condition, NULL); //PTHREAD_COND_INITIALIZER;
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
