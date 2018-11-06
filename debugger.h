#ifndef _DEBUGGER_H_
#define _DEBUGGER_H_

#include "8088.h"

struct registers {
    union {
	uint8_t reg8[8];
	uint16_t reg16[8];
	struct { uint8_t al, ah, cl, ch, dl, dh, bl, bh; };
	struct { uint16_t ax, cx, dx, bx, sp, bp, si, di; };
    };
    union {
	uint16_t segreg[4];
	struct { uint16_t es, cs, ss, ds; };
    };
    uint16_t ip, flags;
};

void copy_cpu_regs(struct iapx88 *cpu, struct registers *regs);

#endif
