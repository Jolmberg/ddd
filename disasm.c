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
          m8 - modregr/m byte 8 bit registers
          m16 - modregr/m byte 16 bit registers
          M8 - modregr/m byte 8 bit registers reverse order
          ms - modregr/m byte segregs
          Ms - modregr/m byte segregs reverse order */
char instr_format[256][20] =
{
    "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
    "", "", "xor $1m8", "xor $1m16", "", "", "", "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
    "jo $1b", "jno $1b", "jb $1b", "jae $1b", "je $1b", "jne $1b", "", "", "js $1b", "jns $1b", "jp $1b", "jnp $1b", "", "", "", "",
    "", "", "", "", "", "", "", "", "", "", "", "mov $1m16", "mov $1Ms", "", "mov $1ms", "",
    "", "", "", "", "", "", "", "", "", "", "", "", "", "", "sahf", "lahf",
    "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
    "mov al, $1i8", "mov cl, $1i8", "mov dl, $1i8", "mov bl, $1i8", "mov ah, $1i8", "mov ch, $1i8", "mov dh, $1i8", "mov bh, $1i8", "mov ax, $1i16", "mov cx, $1i16", "mov dx, $1i16", "mov bx, $1i16", "mov sp, $1i16", "mov bp, $1i16", "mov si, $1i16", "mov di, $1i16",
    "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
    "$1o0 $1n8, 1", "", "$1o0 $1n8, cl", "", "", "", "", "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "", "", "jmp $3i16:$1i16", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "clc", "stc", "cli", "", "", "", "", "$1o1 $1n16"
};

const char extended[2][8][5] =
{ // shifts/rotate
    { "rol", "ror", "rcl", "rcr", "shl", "shr", "", "sar" },
    { "inc", "dec", "call", "call", "jmp", "jmp", "push", "" }
};

const char reg16_name[8][3] = { "ax", "cx", "dx", "bx", "sp", "bp", "si", "di" };
const char reg8_name[8][3] = { "al", "cl", "dl", "bl", "ah", "ch", "dh", "bh" };
const char segreg_name[4][3] = { "es", "cs", "ss", "ds" };

int sprint_instruction_at_address(char *buffer, struct motherboard *mb, uint16_t segment, uint16_t offset) {
    int segment_override = -1;
    uint8_t b = mb_memory_peek(mb, segment, offset);
    if (IS_SEGMENT_OVERRIDE(b)) {
	segment_override = b;
        offset++;
	b = mb_memory_peek(mb, segment, offset);
    }
    char *format = instr_format[mb_memory_peek(mb, segment, offset)];
    int operand_distance = 0;
    int max_distance = 0;
    int modregrm, modxxxrm;
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
                buffer += sprintf(buffer, "0x%x", mb_memory_peek(mb, segment, offset + operand));
                format += 2;
            } else if (!strncmp(format, "i16", 3)) {
                operand_distance++;
                uint16_t word = mb_memory_peek(mb, segment, offset + operand)
                    | (mb_memory_peek(mb, segment, offset + operand + 1) << 8);
                buffer += sprintf(buffer, "0x%x", word);
                format += 3;
            } else if (!strncmp(format, "b", 1)) {
                uint16_t word = offset + 2 + (int8_t)mb_memory_peek(mb, segment, offset + operand);
                buffer += sprintf(buffer, "0x%x", EA(segment, word));
                format += 1;
            } else if (!strncmp(format, "o", 1)) {
                int list = format[1] - '0';
                int number = mb_memory_peek(mb, segment, offset + operand);
                number = (number >> 3) & 7;
                buffer += sprintf(buffer, "%s", extended[list][number]);
                format += 2;
            } else if (!strncmp(format, "n8", 2)) {
                modxxxrm = mb_memory_peek(mb, segment, offset + operand);
                switch(modxxxrm & 0xC0) {
                case 0xC0:
                    buffer += sprintf(buffer, "%s", reg8_name[modxxxrm & 7]);
                    break;
                }
                format += 2;
            } else if (!strncmp(format, "n16", 3)) {
                modxxxrm = mb_memory_peek(mb, segment, offset + operand);
                switch(modxxxrm & 0xC0) {
                case 0xC0:
                    buffer += sprintf(buffer, "%s", reg16_name[modxxxrm & 7]);
                    break;
                }
                format += 3;
            } else if (!strncmp(format, "m8", 2)) {
                modregrm = mb_memory_peek(mb, segment, offset + operand);
                buffer += sprintf(buffer, "%s, ", reg8_name[(modregrm >> 3) & 7]);
                switch (modregrm & 0xC0) {
                case 0xC0:
                    buffer += sprintf(buffer, "%s", reg8_name[modregrm & 7]);
                    break;
                }
                format += 2;
            } else if (!strncmp(format, "m16", 2)) {
                modregrm = mb_memory_peek(mb, segment, offset + operand);
                buffer += sprintf(buffer, "%s, ", reg16_name[(modregrm >> 3) & 7]);
                switch (modregrm & 0xC0) {
                case 0xC0:
                    buffer += sprintf(buffer, "%s", reg16_name[modregrm & 7]);
                    break;
                }
                format += 3;
            } else if (!strncmp(format, "ms", 2)) {
                modregrm = mb_memory_peek(mb, segment, offset + operand);
                buffer += sprintf(buffer, "%s, ", segreg_name[(modregrm >> 3) & 3]);
                switch (modregrm & 0xC0) {
                case 0xC0:
                    buffer += sprintf(buffer, "%s", reg16_name[modregrm & 7]);
                    break;
                }
                format += 2;
            } else if (!strncmp(format, "Ms", 2)) {
                modregrm = mb_memory_peek(mb, segment, offset + operand);
                buffer += sprintf(buffer, "%s, ", reg16_name[modregrm & 7]);
                switch (modregrm & 0xC0) {
                case 0xC0:
                    buffer += sprintf(buffer, "%s", segreg_name[(modregrm >> 3) & 3]);
                    break;
                }
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
    for (line = 0; line < max_lines; line++) {
        address = EA(segment, offset);

        if (address == 0) {
            return line;
	}

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
    }
    return line;
}
