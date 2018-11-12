#ifndef _DISASM_H_
#define _DISASM_H_

#include <stdint.h>

#include "motherboard.h"

int disassemble_from_address(char **buffer, uint32_t *addresses, int *lengths, uint8_t **bytes, struct motherboard *mb, uint16_t segment, uint16_t offset, int max_lines);

#endif
