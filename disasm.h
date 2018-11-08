#ifndef _DISASM_H_
#define _DISASM_H_

#include <stdint.h>

#include "motherboard.h"

int disassemble(char **buffer, struct motherboard *mb, uint32_t address, int max_lines);

#endif
