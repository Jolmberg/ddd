#include <string.h>
#include <stdint.h>

#include "disasm.h"
#include "8088.h"

/* Variable format: $(operand position)(operand type)
   Types: i8 - byte
          i16 - word
          b - branch (cs:(ip + operand))
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
    "", "jno $1b", "", "jae $1b", "", "jne $1b", "", "", "", "jns $1b", "", "jnp $1b", "", "", "", "",
    "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "", "", "", "", "", "", "sahf", "lahf",
    "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
    "mov al, $1i8", "mov cl, $1i8", "mov dl, $1i8", "mov bl, $1i8", "mov ah, $1i8", "mov ch, $1i8", "mov dh, $1i8", "mov bh, $1i8", "mov ax, $1i16", "mov cx, $1i16", "mov dx, $1i16", "mov bx, $1i16", "mov sp, $1i16", "mov bp, $1i16", "mov si, $1i16", "mov di, $1i16",
    "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
    "$1o0 $1n8, 1", "", "$1o0 $1n8, cl", "", "", "", "", "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "", "", "jmp $3i16:$1i16", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "", "", "cli", "", "", "", "", ""
};

const char extended[1][8][5] =
{ // shifts/rotate
    { "rol", "ror", "rcl", "rcr", "shl", "shr", "", "sar" }
};

const char register_name_8[8][3] = { "al", "cl", "dl", "bl", "ah", "ch", "dh", "bh" };

int sprint_instruction_at_address(char *buffer, struct motherboard *mb, uint16_t segment, uint16_t offset) {
    int segment_override = -1;
    uint32_t address = EA(segment, offset);
    uint8_t b = mb_memory_peek(mb, segment, offset);
    if (IS_SEGMENT_OVERRIDE(b)) {
	segment_override = b;
        offset++;
	b = mb_memory_peek(mb, segment, offset);
    }
    char *format = instr_format[mb_memory_peek(mb, segment, offset)];
    int i;
    int operand_distance = 0;
    int max_distance = 0;
    if (format[0] == '\0') {
        if (segment_override >= 0) {
            b = segment_override;
        }
        sprintf(buffer, ".byte %x", b);
        return 1;
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
                int p = sprintf(buffer, "0x%x", mb_memory_peek(mb, segment, offset + operand));
                buffer += p;
                format += 2;
            } else if (!strncmp(format, "i16", 3)) {
                operand_distance++;
                uint16_t word = mb_memory_peek(mb, segment, offset + operand)
                    | (mb_memory_peek(mb, segment, offset + operand + 1) << 8);
                int p = sprintf(buffer, "0x%x", word);
                buffer += p;
                format += 3;
            } else if (!strncmp(format, "b", 1)) {
                uint16_t word = offset + mb_memory_peek(mb, segment, offset + operand) + 2;
                int p = sprintf(buffer, "0x%x", EA(segment, word));
                buffer += p;
                format += 1;
            } else if (!strncmp(format, "o", 1)) {
                int list = format[1] - '0';
                int number = mb_memory_peek(mb, segment, offset + operand);
                number = (number >> 3) & 7;
                int p = sprintf(buffer, "%s", extended[list][number]);
                buffer += p;
                format += 2;
            } else if (!strncmp(format, "n8", 2)) {
                int modxxxrm = mb_memory_peek(mb, segment, offset + operand);
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
    return 1 + max_distance + (segment_override >= 0);
}

int disassemble_from_address(char **buffer, uint32_t *addresses, int *lengths, uint8_t **bytes, struct motherboard *mb, uint16_t segment, uint16_t offset, int max_lines)
{
    int line;
    uint32_t address;
    uint16_t new_offset;
    uint16_t new_segment;
    for (int line = 0; line < max_lines; line++) {
        address = EA(segment, offset);
        addresses[line] = address;
	int length = sprint_instruction_at_address(buffer[line], mb, segment, offset);
	lengths[line] = length;
	for (int i = 0; i < length; i++) {
	    bytes[line][i] = mb_memory_peek(mb, segment, offset + i);
	}

        new_offset = offset + length;

        if (new_offset < offset) {
            new_segment = segment + 0x1000;
            if (new_segment < segment) {
                return line + 1;
            }
            segment = new_segment;
        }
        offset = new_offset;

        if (address == 0) {
            return line + 1;
	}
    }
    return line;
}
