#include <string.h>

#include "debugger.h"
#include "8088.h"

void copy_cpu_regs(struct iapx88 *cpu, struct registers *regs)
{
    memcpy(regs, cpu, sizeof(struct registers));
}
