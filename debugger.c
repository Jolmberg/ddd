#include <string.h>

#if defined(__MACH__)
#include <stdlib.h>
#else
#include <malloc.h>
#endif

#include "debugger.h"
#include "8088.h"

void copy_cpu_regs(struct debugger *d)
{
    struct iapx88 *cpu = d->cpu;
    int index = (d->register_history_start + d->register_history_usage) % d->register_history_size;
    if (d->register_history_usage == d->register_history_size) {
	d->register_history_start = (d->register_history_start + 1) % d->register_history_size;
    } else {
	d->register_history_size++;
    }
    struct registers *regs = d->register_history + index;
    memcpy(regs, cpu, sizeof(struct registers));
}

struct debugger *debugger_create(struct motherboard *mb)
{
    struct debugger *d = (struct debugger *)malloc(sizeof(struct debugger));
    d->mb = mb;
    d->cpu = mb->cpu;
    d->register_history_size = 16;
    d->register_history_usage = 0;
    d->register_history_start = 0;
    d->register_history = (struct registers *)malloc(sizeof(struct registers) * d->register_history_size);
    d->step = mb->step;
    return d;
}

void debugger_step(struct debugger *d)
{
    copy_cpu_regs(d);
    d->step = d->mb->step;
}
