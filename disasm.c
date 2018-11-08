#include <string.h>

#include "disasm.h"

#define IS_SEGMENT_OVERRIDE(x) (((x) & 0xE7) == 0x66)

char instr_format[256][20] =
{
    "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
    "", "", "", "JAE $1i8", "", "JNE $1i8", "", "", "", "JNS $1i8", "", "JNP $1i8", "", "", "", "",
    "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "", "", "", "", "", "", "SAHF", "LAHF",
    "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
    "MOV al, $1i8", "MOV cl, $1i8", "MOV dl, $1i8", "MOV bl, $1i8", "MOV ah, $1i8", "MOV ch, $1i8", "MOV dh, $1i8", "MOV bh, $1i8", "MOV ax, $1i16", "MOV cx, $1i16", "MOV dx, $1i16", "MOV bx, $1i16", "MOV sp, $1i16", "MOV bp, $1i16", "MOV si, $1i16", "MOV di, $1i16",
    "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
    "", "$1o0 $1mr, cl", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "", "", "JMP $3i16:$1i16", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "", "", "CLI", "", "", "", "", ""
};

char extra[][8][5] =
{ // shifts/rotate
    { "ROL", "ROR", "RCL", "RCR", "SHL", "SHR", "", "SAR" }
};



int sprint_instruction_at_address(char *buffer, struct motherboard *mb, uint32_t address) {
    int segment_override = -1;
    uint8_t b = mb_memory_peek(mb, address);
    
    if (IS_SEGMENT_OVERRIDE(b)) {
	segment_override = b;
	b = mb_memory_peek(mb, ++address);
    }
    char *format = instr_format[mb_memory_peek(mb, address)];
    int i;
    int operand_distance = 0;
    int max_distance = 0;
    while (format[0]) {
        if (format[0] != '$') {
            buffer[0] = format[0];
            buffer++;
            format++;
        } else {
            int operand = format[1] - '0';
	    operand_distance = operand;
            format += 2;
            if (!strncmp(format, "i8", 2)) {
                int p = sprintf(buffer, "0x%x", mb_memory_peek(mb, address + operand));
                buffer += p;
                format += 2;
            } else if (!strncmp(format, "i16", 3)) {
		operand_distance++;
                int word = mb_memory_peek(mb, address + operand)
		    | (mb_memory_peek(mb, address +operand + 1) << 8);
                int p = sprintf(buffer, "0x%x", word);
                operand += 2;
                buffer += p;
                format += 3;
            } else {
                format++; // Just get us out of here!
            }
	    if (operand_distance > max_distance) {
		max_distance = operand_distance;
	    }
        }
    }
    buffer[0] = format[0];
    return address + 1 + max_distance;
}

int disassemble_from_address(char **buffer, struct motherboard *mb, uint32_t address, int max_lines)
{
    int line;
    for (int line = 0; line < max_lines; line++) {
	address = sprint_instruction_at_address(buffer[line], mb, address);
	if (address == 0) {
	    return line + 1;
	}
    }
    return line;
}
