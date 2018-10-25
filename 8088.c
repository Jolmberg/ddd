#include <stdio.h>
#include <malloc.h>

#include "8088.h"

#define IS_SEGMENT_OVERRIDE(x) ((x) & 0xE7 == 0x66)

const uint8_t instruction_length[256] =
{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 5, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };

struct iapx88 *iapx88_create(void)
{
    struct iapx88 *cpu = (struct iapx88 *)malloc(sizeof(struct iapx88));
    return cpu;
}

void iapx88_reset(struct iapx88 *cpu)
{
    cpu->flags = 0xF000;
    cpu->ip = 0;
    cpu->cs = 0xFFFF;
    cpu->ds = 0;
    cpu->ss = 0;
    cpu->es = 0;

    cpu->control_bus_state = NONE;
    cpu->prefetch_size = 0;
    cpu->prefetch_offset = 0;
    cpu->state = CPU_FETCH;
    cpu->cur_inst_read = 0;
    cpu->cur_inst_len = 99;
    cpu->segment_override = -1;
    cpu->prefetch_ip = 0;
}

void take_instruction_byte(struct iapx88 *cpu)
{
    uint8_t b = cpu->prefetch_queue[cpu->prefetch_offset];
    cpu->prefetch_offset = (cpu->prefetch_offset + 1) & 3;
    cpu->prefetch_size--;
    if ((cpu->cur_inst_read == 0) && IS_SEGMENT_OVERRIDE(b)) {
	cpu->segment_override = (b >> 3) & 3;
    } else {
	cpu->cur_inst[cpu->cur_inst_read++] = b;
	if (cpu->cur_inst_read == 1) {
	    cpu->cur_inst_len = instruction_length[b];
	}
    }
}

int want_more_instruction_bytes(struct iapx88 *cpu)
{
    return cpu->cur_inst_read < cpu->cur_inst_len;
}

void prefetch_queue_add(struct iapx88 *cpu) {
    int index = (cpu->prefetch_offset + cpu->prefetch_size) & 3;
    cpu->prefetch_queue[index] = cpu->data_pins;
    cpu->prefetch_size++;
}

uint32_t ea(uint16_t segment, uint16_t offset)
{
    return (segment << 4) + offset;
}

uint16_t word_from_bytes(uint8_t *bytes)
{
    return bytes[0] | (bytes[1] << 8);
}

void cleanup(struct iapx88 *cpu)
{
    cpu->cur_inst_len = 99;
    cpu->cur_inst_read = 0;
    cpu->state = CPU_FETCH;
}

int iapx88_step(struct iapx88 *cpu)
{
    uint16_t word1, word2;
    
    if (cpu->control_bus_state == FETCH) {
	prefetch_queue_add(cpu);
	cpu->control_bus_state = NONE;
    }
    while (1) {
        switch (cpu->state) {
        case CPU_FETCH:
	    printf("fetching\n");
	    while (want_more_instruction_bytes(cpu)) {
		if (cpu->prefetch_size) {
		    take_instruction_byte(cpu);
		} else {
		    cpu->control_bus_state = FETCH;
		    printf("Setting address pins: %X\n", ea(cpu->cs, cpu->prefetch_ip));
		    cpu->address_pins = ea(cpu->cs, cpu->prefetch_ip++);
		    return 4;
		}
            }
	    cpu->state = CPU_DECODE;
	    break;
	case CPU_DECODE:
	    printf("decode\n");
	    if (cpu->cur_inst_len == 0) {
		return -1;
	    }
	    switch (cpu->cur_inst[0]) {
	    case 0xEA: /* JMP direct intersegment */
		word1 = word_from_bytes(cpu->cur_inst + 1);
		word2 = word_from_bytes(cpu->cur_inst + 3);
		printf("JMP 0x%X:0x%X\n", word2, word1);
		cpu->cs = word2;
		cpu->ip = word1;
		cpu->prefetch_size = 0;
		cpu->prefetch_ip = cpu->ip;
		cleanup(cpu);
		return 15;
	    case 0xFA: /* CLI */
		printf("CLI\n");
		cpu->flags &= 0xFDFF;
		cleanup(cpu);
		return 2;
		
	    default:
		printf("Unknown opcode: 0x%X\n", cpu->cur_inst[0]);
		return -1;
	    }
	}
    }
}

int run_bus(iapx88 *cpu, int cycles) {
    int c = 0;
    switch (cpu->control_bus_state) {
    case BUS_FETCH:
    case BUS_MEMREAD:
	switch (cpu->bus_state) {
	case BUS_T3:
	    cpu->data_pins = read_memory(cpu->address_pins); // blah
}

int biu_prefetch(struct iapx88 *cpu, int max_cycles)
{
    // BLAH
    if (cpu->control_bus_state == FETCH) {
	prefetch_queue_add(cpu);
	cpu->control_bus_state = NONE;
    }
    if (cpu->prefetch_size < 4) {
	int index = (cpu->prefetch_offset + cpu->prefetch_size) & 3;
	cpu->address_pins = ea(cpu->cs, cpu->prefetch_ip++);
	cpu->control_bus_state = FETCH;
    }
}
