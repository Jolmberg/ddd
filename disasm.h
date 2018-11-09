#ifndef _DISASM_H_
#define _DISASM_H_

#include <stdint.h>

#include "motherboard.h"

int disassemble_from_address(char **buffer, uint32_t *addresses, struct motherboard *mb, uint32_t address, int max_lines);

#endif
