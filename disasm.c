#include <string.h>

#include "disasm.h"

#define IS_SEGMENT_OVERRIDE(x) (((x) & 0xE7) == 0x66)

/* Variable format: $(operand position)(operand type)
   Types: i8 - byte
          i16 - word
          oN - opcode name is in extended list N at index given by the next byte
          n8 - modxxxr/m byte 8 bit registers
          n16 - modxxxr/m byte 16 bit registers
          m - modregr/m byte */
char instr_format[256][20] =
{
    "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
    "", "", "", "jae $1i8", "", "jne $1i8", "", "", "", "jns $1i8", "", "jnp $1i8", "", "", "", "",
    "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "", "", "", "", "", "", "sahf", "lahf",
    "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
    "mov al, $1i8", "mov cl, $1i8", "mov dl, $1i8", "mov bl, $1i8", "mov ah, $1i8", "mov ch, $1i8", "mov dh, $1i8", "mov bh, $1i8", "mov ax, $1i16", "mov cx, $1i16", "mov dx, $1i16", "mov bx, $1i16", "mov sp, $1i16", "mov bp, $1i16", "mov si, $1i16", "mov di, $1i16",
    "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
    "", "", "$1o0 $1n8, cl", "", "", "", "", "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "", "", "jmp $3i16:$1i16", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "", "", "cli", "", "", "", "", ""
};

const char extended[1][8][5] =
{ // shifts/rotate
    { "rol", "ror", "rcl", "rcr", "shl", "shr", "", "sar" }
};

const char register_name_8[8][3] = { "al", "cl", "dl", "bl", "ah", "ch", "dh", "bh" };

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
    if (format[0] == '\0') {
        if (segment_override >= 0) {
            b = segment_override;
        }
        sprintf(buffer, ".byte %x", b);
        return address + 1;
    }
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
                    | (mb_memory_peek(mb, address + operand + 1) << 8);
                int p = sprintf(buffer, "0x%x", word);
                buffer += p;
                format += 3;
            } else if (!strncmp(format, "o", 1)) {
                int list = format[1] - '0';
                int number = mb_memory_peek(mb, address + operand);
                number = (number >> 3) & 7;
                int p = sprintf(buffer, "%s", extended[list][number]);
                buffer += p;
                format += 2;
            } else if (!strncmp(format, "n8", 2)) {
                int modxxxrm = mb_memory_peek(mb, address + operand);
                printf("N8!! addr %x, %x\n", address, modxxxrm);
                int p = 0;
                switch(modxxxrm >> 6) {
                case 3:
                    p = sprintf(buffer, "%s", register_name_8[modxxxrm & 7]);
                    break;
                }
                buffer += p;
                format += 2;
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

int disassemble_from_address(char **buffer, uint32_t *addresses, struct motherboard *mb, uint32_t address, int max_lines)
{
    int line;
    for (int line = 0; line < max_lines; line++) {
        addresses[line] = address;
	address = sprint_instruction_at_address(buffer[line], mb, address);
	if (address == 0) {
	    return line + 1;
	}
    }
    return line;
}
