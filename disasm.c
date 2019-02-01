#include <string.h>
#include <stdint.h>

#include "disasm.h"
#include "8088.h"

/* Variable format: $(operand position)(operand type)
   Types: i8 - byte
          i16 - word
          b8 - branch (cs:(ip + operand)) 8 bit displacement
          b16 - branch (cs:(ip + operand)) 16 bit displacement
          oN - opcode name is in extended list N at index given by the next byte
          n8 - modxxxr/m byte 8 bit registers
          n16 - modxxxr/m byte 16 bit registers
          m8 - modregr/m byte 8 bit registers target r/m
          m16 - modregr/m byte 16 bit registers target r/m
          M8 - modregr/m byte 8 bit registers target register
          M16 - modregr/m byte 16 bit registers target register
          ms - modregr/m byte segregs target r/m
          Ms - modregr/m byte segregs target register */
char instr_format[256][20] =
{
    "add $1m8", "add $1m16", "add $1M8", "add $1M16", "", "", "", "", "or $1m8", "or $1m16", "or $1M8", "or $1M16", "", "", "", "",
    "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
    "and $1m8", "and $1m16", "and $1M8", "and $1M16", "", "", "", "", "sub $1m8", "sub $1m16", "sub $1M8", "sub $1M16", "", "", "", "",
    "xor $1m8", "xor $1m16", "xor $1M8", "xor $1M16", "", "", "", "", "", "", "", "", "", "", "", "",
    "inc ax", "inc cx", "inc dx", "inc bx", "inc sp", "inc bp", "inc si", "inc di", "dec ax", "dec cx", "dec dx", "dec bx", "dec sp", "dec bp", "dec si", "dec di",
    "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
    "jo $1b8", "jno $1b8", "jb $1b8", "jae $1b8", "je $1b8", "jne $1b8", "", "", "js $1b8", "jns $1b8", "jp $1b8", "jnp $1b8", "", "", "", "",
    "", "", "", "", "", "", "", "", "", "", "mov $1M8", "mov $1M16", "mov $1ms", "", "mov $1Ms", "",
    "", "", "", "", "", "", "", "", "", "", "", "", "", "", "sahf", "lahf",
    "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
    "mov al, $1i8", "mov cl, $1i8", "mov dl, $1i8", "mov bl, $1i8", "mov ah, $1i8", "mov ch, $1i8", "mov dh, $1i8", "mov bh, $1i8", "mov ax, $1i16", "mov cx, $1i16", "mov dx, $1i16", "mov bx, $1i16", "mov sp, $1i16", "mov bp, $1i16", "mov si, $1i16", "mov di, $1i16",
    "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
    "$1o0 $1n8, 1", "", "$1o0 $1n8, cl", "", "", "", "", "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "out $1i8, al", "out $1i8, ax", "", "jmp $1b16", "jmp $3i16:$1i16", "", "", "", "out dx, al", "out dx, ax",
    "", "", "", "", "", "", "", "", "clc", "stc", "cli", "", "", "", "$1o1 $1n8", "$1o1 $1n16"
};

const char extended[2][8][5] =
{
    { "rol", "ror", "rcl", "rcr", "shl", "shr", "", "sar" },
    { "inc", "dec", "call", "call", "jmp", "jmp", "push", "" }
};

const char reg_name[3][8][3] = { { "al", "cl", "dl", "bl", "ah", "ch", "dh", "bh" },
                                 { "ax", "cx", "dx", "bx", "sp", "bp", "si", "di" },
                                 { "es", "cs", "ss", "ds" } };

const char rm_string[8][8] = { "[bx+si]", "[bx+di]", "[bp+si]", "[bp+di]",
                               "[si]", "[di]", "[bp]", "[bx]" };

int sprint_modregrm_string(char *buffer, uint8_t rm, int regtype, int reg, int reverse, int override)
{
    char ptr[2][5] = { "BYTE", "WORD" };
    char *orgbuffer = buffer;
    int rmtype = regtype == 2 ? 1 : regtype;
    if (reg && reverse) {
        buffer += sprintf(buffer, "%s, ", reg_name[regtype][(rm >> 3) & 7]);
    }
    switch(rm & 0xC0) {
    case 0xC0:
        buffer += sprintf(buffer, "%s", reg_name[rmtype][rm & 7]);
        break;
    case 0:
        buffer += sprintf(buffer, "%s PTR ", ptr[rmtype]);
        if (override != -1) {
            buffer += sprintf(buffer, "%s:", reg_name[2][(override >> 3) & 3]);
        }
        buffer += sprintf(buffer, "%s", rm_string[rm & 7]);
        break;
    }
    if (reg && !reverse) {
        buffer += sprintf(buffer, ", %s", reg_name[regtype][(rm >> 3) & 7]);
    }

    return buffer - orgbuffer;
}


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
            } else if (!strncmp(format, "b8", 2)) {
                uint16_t word = offset + 2 + (int8_t)mb_memory_peek(mb, segment, offset + operand);
                buffer += sprintf(buffer, "0x%x", EA(segment, word));
                format += 2;
            } else if (!strncmp(format, "b16", 3)) {
                operand_distance++;
                uint16_t word = offset + 3 + (int16_t)(mb_memory_peek(mb, segment, offset + operand) + 256 * mb_memory_peek(mb, segment, offset + operand + 1));
                buffer += sprintf(buffer, "0x%x", EA(segment, word));
                format += 3;
            } else if (!strncmp(format, "o", 1)) {
                int list = format[1] - '0';
                int number = mb_memory_peek(mb, segment, offset + operand);
                number = (number >> 3) & 7;
                buffer += sprintf(buffer, "%s", extended[list][number]);
                format += 2;
            } else if (!strncmp(format, "n8", 2)) {
                modxxxrm = mb_memory_peek(mb, segment, offset + operand);
                buffer += sprint_modregrm_string(buffer, modxxxrm, 0, 0, 0, segment_override);
                format += 2;
            } else if (!strncmp(format, "n16", 3)) {
                modxxxrm = mb_memory_peek(mb, segment, offset + operand);
                buffer += sprint_modregrm_string(buffer, modregrm, 1, 0, 0, segment_override);
                format += 3;
            } else if (!strncmp(format, "m8", 2)) {
                modregrm = mb_memory_peek(mb, segment, offset + operand);
                buffer += sprint_modregrm_string(buffer, modregrm, 0, 1, 0, segment_override);
                format += 2;
            } else if (!strncmp(format, "m16", 3)) {
                modregrm = mb_memory_peek(mb, segment, offset + operand);
                buffer += sprint_modregrm_string(buffer, modregrm, 1, 1, 0, segment_override);
                format += 3;
            } else if (!strncmp(format, "M8", 2)) {
                modregrm = mb_memory_peek(mb, segment, offset + operand);
                buffer += sprint_modregrm_string(buffer, modregrm, 0, 1, 1, segment_override);
                format += 2;
            } else if (!strncmp(format, "M16", 3)) {
                modregrm = mb_memory_peek(mb, segment, offset + operand);
                buffer += sprint_modregrm_string(buffer, modregrm, 1, 1, 1, segment_override);
                format += 3;
            } else if (!strncmp(format, "ms", 2)) {
                modregrm = mb_memory_peek(mb, segment, offset + operand);
                buffer += sprint_modregrm_string(buffer, modregrm, 2, 1, 0, segment_override);
                format += 2;
            } else if (!strncmp(format, "Ms", 2)) {
                modregrm = mb_memory_peek(mb, segment, offset + operand);
                buffer += sprint_modregrm_string(buffer, modregrm, 2, 1, 1, segment_override);
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
