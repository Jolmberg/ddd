#include "disasm.h"

#define IS_SEGMENT_OVERRIDE(x) (((x) & 0xE7) == 0x66)

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

int disassemble(char **buffer, struct motherboard *mb, uint32_t address, int max_lines)
{
    
}


void sprint_instruction(char *buffer, uint32_t address) {
    int segment_override = -1;
    uint8_t b = mb_memory_peek(mb, address++);
    
    if (IS_SEGMENT_OVERRIDE(b)) {
	segment_override = b;
	b = mb_memory_peek(mb, address++);
    }
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

