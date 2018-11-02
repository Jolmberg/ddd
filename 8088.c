#include <stdio.h>
#include <string.h>

#if defined(__MACH__)
#include <stdlib.h>
#else
#include <malloc.h>
#endif

#include "8088.h"

#define IS_SEGMENT_OVERRIDE(x) (((x) & 0xE7) == 0x66)

const uint8_t instruction_length[256] =
{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, // mov reg, immediate
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 5, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };


char instruction_format[256][20] =
{
    "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
    "", "", "", "JAE $1i8", "", "", "", "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "", "", "", "", "", "", "SAHF", "",
    "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
    "MOV al, $1i8", "MOV cl, $1i8", "MOV dl, $1i8", "MOV bl, $1i8", "MOV ah, $1i8", "MOV ch, $1i8", "MOV dh, $1i8", "MOV bh, $1i8", "MOV ax, $1i16", "MOV cx, $1i16", "MOV dx, $1i16", "MOV bx, $1i16", "MOV sp, $1i16", "MOV bp, $1i16", "MOV si, $1i16", "MOV di, $1i16",
    "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "", "", "JMP $3i16:$1i16", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "", "", "CLI", "", "", "", "", ""
};


void sprint_instruction(char *buffer, struct iapx88 *cpu) {
    char *format = instruction_format[cpu->cur_inst[0]];
    int i;
    while (format[0]) {
        if (format[0] != '$') {
            buffer[0] = format[0];
            buffer++;
            format++;
        } else {
            int operand = format[1] - '0';
            format += 2;
            if (!strncmp(format, "i8", 2)) {
                int p = sprintf(buffer, "0x%x", cpu->cur_inst[operand]);
                buffer += p;
                format += 2;
            } else if (!strncmp(format, "i16", 3)) {
                int word = cpu->cur_inst[operand] | (cpu->cur_inst[operand + 1] << 8);
                int p = sprintf(buffer, "0x%x", word);
                operand += 2;
                buffer += p;
                format += 3;
            } else {
                format++; // Just get us out of here!
            }
        }
    }
    buffer[0] = format[0];
}

void print_instruction(struct iapx88 *cpu)
{
    char buffer[30];
    sprint_instruction(buffer, cpu);
    printf("%s\n", buffer);
}

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

    cpu->control_bus_state = BUS_NONE;
    cpu->prefetch_size = 0;
    cpu->prefetch_offset = 0;
    cpu->state = CPU_FETCH;
    cpu->cur_inst_read = 0;
    cpu->cur_inst_len = 99;
    cpu->segment_override = -1;
    cpu->prefetch_ip = 0;
    cpu->prefetch_forbidden = 0;
    cpu->return_reason = WAIT_INTERRUPTIBLE;
}

void check_segment_override(struct iapx88 *cpu, uint8_t b)
{
    if ((cpu->cur_inst_read == 0) && IS_SEGMENT_OVERRIDE(b)) {
	cpu->segment_override = (b >> 3) & 3;
    } else {
	cpu->cur_inst[cpu->cur_inst_read++] = b;
	if (cpu->cur_inst_read == 1) {
	    cpu->cur_inst_len = instruction_length[b];
	}
    }
}

void take_instruction_byte_from_prefetch(struct iapx88 *cpu)
{
    uint8_t b = cpu->prefetch_queue[cpu->prefetch_offset];
    cpu->prefetch_offset = (cpu->prefetch_offset + 1) & 3;
    cpu->prefetch_size--;
    check_segment_override(cpu, b);
}

void take_instruction_byte_from_biu(struct iapx88 *cpu)
{
    uint8_t b = cpu->eu_biu_byte;
    check_segment_override(cpu, b);
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
    while (1) {
        switch (cpu->state) {
        case CPU_FETCH:
	    printf("fetching\n");
	    while (want_more_instruction_bytes(cpu)) {
		if (cpu->prefetch_size > 0) {
                    printf("Yay, prefetched!\n");
		    take_instruction_byte_from_prefetch(cpu);
		} else {
		    if (cpu->return_reason == WAIT_BIU && cpu->eu_wanted_control_bus_state == BUS_FETCH) {
                        cpu->return_reason = NO_REASON;
			take_instruction_byte_from_biu(cpu);
		    } else {
			cpu->return_reason = WAIT_BIU;
			cpu->eu_wanted_control_bus_state = BUS_FETCH;
			cpu->eu_wanted_segment = cpu->cs;
			cpu->eu_wanted_offset = cpu->prefetch_ip++;
			return 0;
		    }
		}
            }
	    cpu->state = CPU_DECODE;
	    break;
	case CPU_DECODE:
	    printf("decode\n");
            printf("Instruction: ");
            print_instruction(cpu);
	    if (cpu->cur_inst_len == 0) {
		return -1;
	    }
	    switch (cpu->cur_inst[0]) {
	    case 0xEA: /* JMP direct intersegment */
		word1 = word_from_bytes(cpu->cur_inst + 1);
		word2 = word_from_bytes(cpu->cur_inst + 3);
                print_instruction(cpu);
		cpu->cs = word2;
		cpu->ip = word1;
		cpu->prefetch_size = 0;
		cpu->prefetch_ip = cpu->ip;
                cpu->prefetch_forbidden = 1;
		cleanup(cpu);
		cpu->return_reason = WAIT_INTERRUPTIBLE;
		return 15;
	    case 0xFA: /* CLI */
		cpu->flags &= 0xFDFF;
		cleanup(cpu);
		cpu->return_reason = WAIT_INTERRUPTIBLE;
		return 2;
            case 0xB0:
            case 0xB1:
            case 0xB2:
            case 0xB3:
            case 0xB4:
            case 0xB5:
            case 0xB6:
            case 0xB7: /* MOV reg8, immediate */
		cpu->reg = cpu->cur_inst[0] & 3;
                cpu->reg8[cpu->reg] = cpu->cur_inst[1];
                cleanup(cpu);
		cpu->return_reason = WAIT_INTERRUPTIBLE;
                return 4;
            case 0x9E: /* SAHF */
                cpu->flags = (cpu->flags & 0xFF2A) | (cpu->ah & 0xD5);
                cleanup(cpu);
		cpu->return_reason = WAIT_INTERRUPTIBLE;
                return 4;
	    default:
		printf("Unknown opcode: 0x%X\n", cpu->cur_inst[0]);
		return -1;
	    }
	}
    }
}

/* int run_bus(iapx88 *cpu, int cycles) { */
/*     int c = 0; */
/*     switch (cpu->control_bus_state) { */
/*     case BUS_FETCH: */
/*     case BUS_MEMREAD: */
/* 	switch (cpu->bus_state) { */
/* 	case BUS_T3: */
/* 	    cpu->data_pins = read_memory(cpu->address_pins); // blah */
/* } */

int biu_request_prefetch(struct iapx88 *cpu, int max_cycles)
{
    if (cpu->prefetch_size == 4) {
	return max_cycles;
    }
    switch (cpu->bus_state) {
    case BUS_IDLE:
	cpu->control_bus_state = BUS_FETCH;
	cpu->bus_state = BUS_T3;
	cpu->address_pins = ea(cpu->cs, cpu->prefetch_ip++);
	return 3;
    case BUS_T1:
	cpu->bus_state = BUS_T3;
	return 2;
    case BUS_T2:
	cpu->bus_state = BUS_T3;
	return 1;
    case BUS_T3:
	return 0;
    case BUS_T4:
	cpu->eu_biu_byte = cpu->data_pins;
	cpu->bus_state = BUS_IDLE;
	cpu->control_bus_state = BUS_NONE;
	return 1;
    case BUS_TW:
	return 1;
    }
}

int biu_handle_prefetch(struct iapx88 *cpu)
{
    int index = (cpu->prefetch_offset + cpu->prefetch_size) & 3;
    cpu->prefetch_queue[index] = cpu->data_pins;
    cpu->prefetch_size++;
    cpu->bus_state = BUS_IDLE;
    cpu->control_bus_state = BUS_NONE;
    return 1;
}

int biu_make_request(struct iapx88 *cpu)
{
    cpu->control_bus_state = cpu->eu_wanted_control_bus_state;
    cpu->address_pins = ea(cpu->eu_wanted_segment, cpu->eu_wanted_offset);
    if (cpu->control_bus_state == BUS_MEMWRITE) {
	cpu->data_pins = cpu->eu_biu_byte;
    }
    cpu->bus_state = BUS_T3;
    return 3;
}

int biu_handle_response(struct iapx88 *cpu)
{
    if (cpu->control_bus_state == BUS_FETCH || cpu->control_bus_state == BUS_MEMREAD) {
	cpu->eu_biu_byte = cpu->data_pins;
    }
    cpu->control_bus_state = BUS_NONE;
    cpu->bus_state = BUS_IDLE;
    return 1;
}
