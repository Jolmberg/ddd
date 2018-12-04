#ifndef _DEBUGGER_H_
#define _DEBUGGER_H_

#include "8088.h"
#include "motherboard.h"

#define DISASM_LINE_LENGTH 40

struct registers {
    union {
	uint8_t reg8[8];
	struct { uint8_t al, ah, cl, ch, dl, dh, bl, bh; };
	uint16_t reg16[12];
        struct { uint16_t dummy[8], segreg[4]; };
	struct { uint16_t ax, cx, dx, bx, sp, bp, si, di, es, cs, ss, ds; };
    };
    uint16_t ip, flags;
};

struct debugger {
    struct motherboard *mb;
    struct iapx88 *cpu;
    struct registers *register_history;
    int register_history_size, register_history_start, register_history_usage;
    int paused;

    int disassembly_lines;
    char *disassembly[100];
    uint32_t disassembly_addresses[100];
    int lengths[100];
    uint8_t *bytes[100];

    uint32_t breakpoint;

    int step;
};

struct debugger *debugger_create(struct motherboard *mb);
void *debugger_run(void *debugger);
void debugger_step(struct debugger *debugger);
struct registers *debugger_get_cpu_regs(struct debugger *d, int boffset);


#endif
