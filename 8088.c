#include <stdio.h>
#include <string.h>

#if defined(__MACH__)
#include <stdlib.h>
#else
#include <malloc.h>
#endif

#include "8088.h"

const uint8_t instruction_length[256] =
{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1,
  1, 1, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  2, 2, 2, 2, 2, 2, 1, 1, 2, 2, 2, 2, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, // mov reg, immediate
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  2, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 2, 2, 1, 3, 5, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 1 };

#define UGH {2, MOD_NONE, TARGET_NONE}
#define BNN {0, MOD_NONE, TARGET_NONE}
#define WNN {1, MOD_NONE, TARGET_NONE}
#define BMN {0, MOD_REGRM, TARGET_NONE}
#define BMR {0, MOD_REGRM, TARGET_RM}
#define WMN {1, MOD_REGRM, TARGET_NONE}
#define BRN {0, REG_REG, TARGET_NONE}
#define WRN {1, REG_REG, TARGET_NONE}
#define BXN {0, MOD_XXXRM, TARGET_NONE}
#define BXR {0, MOD_XXXRM, TARGET_RM}
#define WSN {1, MOD_SEGRM, TARGET_NONE}
#define WSR {1, MOD_SEGRM, TARGET_RM}

struct instruction_desc description[256] =
{ UGH, UGH, UGH, UGH, UGH, UGH, UGH, UGH, UGH, UGH, UGH, WMN, UGH, UGH, UGH, UGH,
  UGH, UGH, UGH, UGH, UGH, UGH, UGH, UGH, UGH, UGH, UGH, UGH, UGH, UGH, UGH, UGH,
  BMR, UGH, UGH, UGH, UGH, UGH, UGH, UGH, UGH, UGH, BMN, UGH, UGH, UGH, UGH, UGH,
  UGH, UGH, BMN, WMN, UGH, UGH, UGH, UGH, UGH, UGH, UGH, UGH, UGH, UGH, UGH, UGH,
  UGH, UGH, UGH, UGH, UGH, UGH, UGH, UGH, BRN, BRN, BRN, BRN, BRN, BRN, BRN, BRN,
  UGH, UGH, UGH, UGH, UGH, UGH, UGH, UGH, UGH, UGH, UGH, UGH, UGH, UGH, UGH, UGH,
  UGH, UGH, UGH, UGH, UGH, UGH, UGH, UGH, UGH, UGH, UGH, UGH, UGH, UGH, UGH, UGH,
  BNN, BNN, BNN, BNN, BNN, BNN, UGH, UGH, BNN, BNN, BNN, BNN, UGH, UGH, UGH, UGH,
  UGH, UGH, UGH, UGH, UGH, UGH, UGH, UGH, UGH, UGH, BMN, WMN, WSR, UGH, WSN, UGH,
  UGH, UGH, UGH, UGH, UGH, UGH, UGH, UGH, UGH, UGH, UGH, UGH, UGH, UGH, BNN, BNN,
  UGH, UGH, UGH, UGH, UGH, UGH, UGH, UGH, UGH, UGH, UGH, UGH, UGH, UGH, UGH, UGH,
  BRN, BRN, BRN, BRN, BRN, BRN, BRN, BRN, WRN, WRN, WRN, WRN, WRN, WRN, WRN, WRN,
  UGH, UGH, UGH, UGH, UGH, UGH, UGH, UGH, UGH, UGH, UGH, UGH, UGH, UGH, UGH, UGH,
  BXR, UGH, BXR, UGH, UGH, UGH, UGH, UGH, UGH, UGH, UGH, UGH, UGH, UGH, UGH, UGH,
  UGH, UGH, UGH, UGH, UGH, UGH, BNN, WNN, UGH, WNN, BNN, UGH, UGH, UGH, BNN, WNN,
  UGH, UGH, UGH, UGH, UGH, UGH, UGH, UGH, BNN, BNN, BNN, UGH, UGH, UGH, BXR, UGH };

#undef UGH
#undef BNN
#undef WNN
#undef BMN
#undef BMR
#undef WMN
#undef BRN
#undef WRN
#undef BXN
#undef BXR
#undef WSN
#undef WSR

int fetch(struct iapx88 *cpu);
int decode(struct iapx88 *cpu);
int execute(struct iapx88 *cpu);

struct iapx88 *iapx88_create(void)
{
    struct iapx88 *cpu = (struct iapx88 *)malloc(sizeof(struct iapx88));
    return cpu;
}

void iapx88_reset(struct iapx88 *cpu)
{
    cpu->flags = 0xF000;
    cpu->flag_pf_source = 0;
    cpu->ip = 0;
    cpu->cs = 0xFFFF;
    cpu->ds = 0;
    cpu->ss = 0;
    cpu->es = 0;

    cpu->control_bus_state = BUS_NONE;
    cpu->prefetch_usage = 0;
    cpu->prefetch_start = 0;
    cpu->state = CPU_FETCH;
    cpu->cur_inst_read = 0;
    cpu->cur_inst_len = 99;
    cpu->segment_override = -1;
    cpu->prefetch_ip = 0;
    cpu->prefetch_forbidden = 0;
    cpu->jumped = 0;
    cpu->return_reason = WAIT_INTERRUPTIBLE;
    cpu->next_step = fetch;
}

void check_segment_override(struct iapx88 *cpu, uint8_t b)
{
    if ((cpu->cur_inst_read == 0) && IS_SEGMENT_OVERRIDE(b)) {
	cpu->segment_override = (b >> 3) & 3;
    } else {
	cpu->cur_inst[cpu->cur_inst_read++] = b;
	if (cpu->cur_inst_read == 1) {
	    cpu->cur_inst_len = instruction_length[b];
	}
    }
}

void take_instruction_byte_from_prefetch(struct iapx88 *cpu)
{
    uint8_t b = cpu->prefetch_queue[cpu->prefetch_start];
    cpu->prefetch_start = (cpu->prefetch_start + 1) & 3;
    cpu->prefetch_usage--;
    check_segment_override(cpu, b);
}

void take_instruction_byte_from_biu(struct iapx88 *cpu)
{
    uint8_t b = cpu->eu_biu_byte;
    check_segment_override(cpu, b);
}

int want_more_instruction_bytes(struct iapx88 *cpu)
{
    return cpu->cur_inst_read < cpu->cur_inst_len;
}

void prefetch_queue_add(struct iapx88 *cpu) {
    int index = (cpu->prefetch_start + cpu->prefetch_usage) & 3;
    cpu->prefetch_queue[index] = cpu->data_pins;
    cpu->prefetch_usage++;
}

uint16_t word_from_bytes(uint8_t *bytes)
{
    return bytes[0] | (bytes[1] << 8);
}

// An instruction has finished executing, reset the state machine
int cleanup(struct iapx88 *cpu)
{
    printf("cleanup\n");
    if (cpu->jumped) {
        cpu->prefetch_usage = 0;
        cpu->prefetch_ip = cpu->ip;
        cpu->prefetch_forbidden = 1;
        cpu->jumped = 0;
    } else {
	cpu->ip += cpu->cur_inst_len + (cpu->segment_override >= 0);
    }
    cpu->cur_inst_len = 99;
    cpu->cur_inst_read = 0;
    cpu->segment_override = -1;
    cpu->next_step = fetch;
    cpu->state = CPU_FETCH;
    cpu->return_reason = WAIT_INTERRUPTIBLE;
    return 0;
}

int branch(struct iapx88 *cpu, int taken)
{
    int cycles = 4;
    if (taken) {
        cpu->ip = cpu->ip + 2 + (int8_t)cpu->cur_inst[1];
        cycles += 12;
        cpu->jumped = 1;
    }
    return cycles;
}

void set_flag(struct iapx88 *cpu, uint16_t flag, int state)
{
    if (state) {
        cpu->flags |= flag;
    } else {
        cpu->flags &= (0xFFFF ^ flag);
    }
}

void iapx88_update_flag_pf(struct iapx88 *cpu)
{
    set_flag(cpu, FLAG_PF, __builtin_parity(cpu->flag_pf_source)); // GCC magic
}

void set_flags_pf_zf_sf_8(struct iapx88 *cpu, uint8_t result)
{
    cpu->flag_pf_source = result;
    set_flag(cpu, FLAG_ZF, !result);
    set_flag(cpu, FLAG_SF, result & 0x80);
}

void set_flags_from_bitwise8(struct iapx88 *cpu, uint8_t result)
{
    set_flag(cpu, FLAG_CF, 0);
    cpu->flag_pf_source = result;
    set_flag(cpu, FLAG_ZF, !result);
    set_flag(cpu, FLAG_SF, result & 0x80);
    set_flag(cpu, FLAG_OF, 0);
}

void set_flags_from_bitwise16(struct iapx88 *cpu, uint16_t result)
{
    set_flag(cpu, FLAG_CF, 0);
    cpu->flag_pf_source = result;
    set_flag(cpu, FLAG_ZF, !result);
    set_flag(cpu, FLAG_SF, result & 0x8000);
    set_flag(cpu, FLAG_OF, 0);
}

int execute(struct iapx88 *cpu)
{
    uint8_t temp8;
    int wait_for_bus = 0;
    printf("execute!!\n");
    //cpu->next_step = execute;
    int cycles = 0;
    switch (cpu->cur_inst[0]) {
    case 0x0b: /* or modregrm (to reg16) */
        *cpu->operand_reg8 |= *cpu->operand_rm8;
        set_flags_from_bitwise8(cpu, *cpu->operand_rm8);
        cycles = 3;
        break;
    case 0x20: /* and modregrm (from reg8) */
        *cpu->operand_rm8 &= *cpu->operand_reg8;
        set_flags_from_bitwise8(cpu, *cpu->operand_rm8);
        cycles = 3;
        break;
    case 0x2a: /* sub modregrm (to reg8) */
        set_flag(cpu, FLAG_AF, ((*cpu->operand_reg8 & 0xF) < (*cpu->operand_rm8 & 0xF)));
        temp8 = *cpu->operand_reg8 - *cpu->operand_rm8;
        set_flag(cpu, FLAG_CF, *cpu->operand_rm8 > *cpu->operand_reg8);
        set_flag(cpu, FLAG_OF, ((*cpu->operand_reg8 & 0x80) ^ (*cpu->operand_rm8 & 0x80)) & ((temp8 & 0x80) ^ (*cpu->operand_reg8)));
        set_flags_pf_zf_sf_8(cpu, temp8);
        *cpu->operand_reg8 = temp8;
        cycles = 3;
        break;
    case 0x30: /* xor modregrm (from reg8) */
        *cpu->operand_rm8 ^= *cpu->operand_reg8;
        set_flags_from_bitwise8(cpu, *cpu->operand_rm8);
        cycles = 3;
        break;
    case 0x32: /* xor modregrm (to reg8) */
        *cpu->operand_reg8 ^= *cpu->operand_rm8;
        set_flags_from_bitwise8(cpu, *cpu->operand_reg8);
        cycles = 3;
        break;
    case 0x31: /* xor modregrm (from reg16) */
        *cpu->operand_rm16 ^= *cpu->operand_reg16;
        set_flags_from_bitwise16(cpu, *cpu->operand_rm16);
        cycles = 3;
        break;
    case 0x33: /* xor modregrm (to reg16) */
        *cpu->operand_reg16 ^= *cpu->operand_rm16;
        set_flags_from_bitwise16(cpu, *cpu->operand_reg16);
        cycles = 3;
        break;
    case 0x48: /* dec reg */
    case 0x49:
    case 0x4a:
    case 0x4b:
    case 0x4c:
    case 0x4d:
    case 0x4e:
    case 0x4f:
        set_flag(cpu, FLAG_AF, (*cpu->operand_reg8 & 0x0F) == 0);
        set_flag(cpu, FLAG_OF, *cpu->operand_reg8 == 0x80);
        (*cpu->operand_reg8)--;
        set_flags_pf_zf_sf_8(cpu, *cpu->operand_reg8);
        cycles = 3;
    case 0x70: /* branches */
        cycles = branch(cpu, cpu->flags & FLAG_OF); // JO
        break;
    case 0x71:
        cycles = branch(cpu, !(cpu->flags & FLAG_OF)); // JNO
        break;
    case 0x72:
        cycles = branch(cpu, cpu->flags & FLAG_CF); // JB
        break;
    case 0x73:
        cycles = branch(cpu, !(cpu->flags & FLAG_CF)); // JAE
        break;
    case 0x74:
        cycles = branch(cpu, cpu->flags & FLAG_ZF); // JE
        break;
    case 0x75:
        cycles = branch(cpu, !(cpu->flags & FLAG_ZF)); // JNE
        break;
    case 0x78:
        cycles = branch(cpu, cpu->flags & FLAG_SF); // JS
        break;
    case 0x79:
        cycles = branch(cpu, !(cpu->flags & FLAG_SF)); // JNS
        break;
    case 0x7a:
        iapx88_update_flag_pf(cpu);
        cycles = branch(cpu, cpu->flags & FLAG_PF); // JP
        break;
    case 0x7b:
        iapx88_update_flag_pf(cpu);
        cycles = branch(cpu, !(cpu->flags & FLAG_PF)); // JNP
        break;
    case 0x8a: /* mov modregr/m to reg8 */
        *cpu->operand_reg8 = *cpu->operand_rm8;
        cycles = 2;
        break;
    case 0x8b: /* MOV modregr/m to reg16 */
        *cpu->operand_reg16 = *cpu->operand_rm16;
        cycles = 2;
        break;
    case 0x8c: /* MOV modregr/m from segreg */
        *cpu->operand_rm16 = *cpu->operand_reg16;
        cycles = 2;
        break;
    case 0x8e: /* MOV modregr/m to segreg */
        *cpu->operand_reg16 = *cpu->operand_rm16;
        cycles = 2;
        break;
    case 0x9E: /* SAHF */
        cpu->flags = (cpu->flags & 0xFF2A) | (cpu->ah & 0xD5);
        cpu->flag_pf_source = cpu->flags & FLAG_PF;
        cycles = 4;
        break;
    case 0x9F: /* LAHF */
        iapx88_update_flag_pf(cpu);
        cpu->ah = (cpu->ah & 0x2A) | (cpu->flags & 0xD5);
        cycles = 4;
        break;
    case 0xB0: /* MOV reg8, immediate */
    case 0xB1:
    case 0xB2:
    case 0xB3:
    case 0xB4:
    case 0xB5:
    case 0xB6:
    case 0xB7:
        *cpu->operand_reg8 = cpu->cur_inst[1];
        cycles = 4;
        break;
    case 0xB8: /* MOV reg16, immediate */
    case 0xB9:
    case 0xBA:
    case 0xBB:
    case 0xBC:
    case 0xBD:
    case 0xBE:
    case 0xBF:
        *cpu->operand_reg16 = word_from_bytes(cpu->cur_inst + 1);
        cycles = 4;
        break;
    case 0xD0: /* shift/rotate by 1 */
        switch(cpu->cur_inst[1] & 0x38) {
        case 0x20: /* SHL modxxxrm, 1 */
            set_flag(cpu, FLAG_CF, (*cpu->operand_rm8 & 0x80));
            temp8 = *cpu->operand_rm8 << 1;
            set_flag(cpu, FLAG_OF, (*cpu->operand_rm8 & 0x80) ^ (temp8 & 0x80));
            *cpu->operand_rm8 = temp8;
            set_flags_pf_zf_sf_8(cpu, *cpu->operand_rm8);
            cycles = 2;
            break;
        }
        break;
    case 0xD2: /* shift/rotate by cl */
        switch(cpu->cur_inst[1] & 0x38) {
        case 0x28: /* SHR modxxxrm, cl */
            cycles = 8 + 4 * cpu->cl; // Set cycles here because cl might change
            if (cpu->cl > 0) {
                *cpu->operand_rm8 >>= (cpu->cl - 1);
                set_flag(cpu, FLAG_CF, *cpu->operand_rm8 & 1);
                *cpu->operand_rm8 >>= 1;
                set_flags_pf_zf_sf_8(cpu, *cpu->operand_rm8);
            }
            break;
        }
        break;
    case 0xE6: /* out immediate, al */
        cpu->eu_wanted_control_bus_state = BUS_IOWRITE;
        cpu->eu_wanted_port = cpu->cur_inst[1];
        cpu->eu_biu_byte = cpu->al;
        cycles = 10;
        cpu->return_reason = WAIT_BIU;
        wait_for_bus = 1;
        break;
    case 0xE7: /* out immediate, ax */
        cpu->eu_wanted_control_bus_state = BUS_IOWRITE;
        cpu->eu_wanted_port = cpu->cur_inst[1];
        cpu->eu_biu_byte = cpu->ax;
        cycles = 10;
        cpu->return_reason = WAIT_BIU;
        wait_for_bus = 1;
        break;
    case 0xEE: /* out dx, al */
        cpu->eu_wanted_control_bus_state = BUS_IOWRITE;
        cpu->eu_wanted_port = cpu->dx;
        cpu->eu_biu_byte = cpu->al;
        cycles = 8;
        cpu->return_reason = WAIT_BIU;
        wait_for_bus = 1;
        break;
    case 0xE9: /* jmp immediate intrasegment */
        cpu->ip += word_from_bytes(cpu->cur_inst + 1) + 3;
        cpu->jumped = 1;
        cycles = 15;
        break;
    case 0xEF: /* out dx, ax */
        cpu->eu_wanted_control_bus_state = BUS_IOWRITE;
        cpu->eu_wanted_port = cpu->dx;
        cpu->eu_biu_byte = cpu->ax;
        cycles = 8;
        cpu->return_reason = WAIT_BIU;
        wait_for_bus = 1;
        break;
    case 0xEA: /* JMP direct intersegment */
        cpu->cs = word_from_bytes(cpu->cur_inst + 3);
        cpu->ip = word_from_bytes(cpu->cur_inst + 1);
        cpu->jumped = 1;
        cycles = 15;
        break;
    case 0xF8: /* CLC */
        set_flag(cpu, FLAG_CF, 0);
        cycles = 2;
        break;
    case 0xF9: /* STC */
        set_flag(cpu, FLAG_CF, 1);
        cycles = 2;
        break;
    case 0xFA: /* CLI */
        set_flag(cpu, FLAG_IF, 0);
        cycles = 2;
        break;
        /* case 0xFF: /\* PUSH, CALL, JMP, INC, DEC modxxxr/m *\/ */
        /* 	modregrm = cpu->cur_inst[1]; */
        /* 	switch (modregrm & 0x38) { */
        /* 	case 0x30: */
        /* 	    cpu->reg1 = modregrm & 7; */
        /* 	} */
    case 0xFE: /* INC, DEC, CALL, JMP, PUSH modxxxr/m */
        switch (cpu->cur_inst[1] & 0x38) {
        case 0x00: /* INC */
            set_flag(cpu, FLAG_OF, *cpu->operand_rm8 == 0x7F);
            set_flag(cpu, FLAG_AF, (*cpu->operand_rm8 & 0x0F) == 0x0F);
            (*cpu->operand_rm8)++;
            set_flags_pf_zf_sf_8(cpu, *cpu->operand_rm8);
            cycles = 3;
            break;
        default:
            printf("Unhandled variant of 0x%x\n", cpu->cur_inst[0]);
        }
        break;
    default:
        printf("Unknown opcode: 0x%X\n", cpu->cur_inst[0]);
        return -1;
    }

    if (wait_for_bus) {
        cpu->next_step = cleanup;
        return cycles;
    }
    cleanup(cpu);
    return cycles;
}

int decode(struct iapx88 *cpu)
{
    printf("decode!!!!\n");
    int reg, rm;
    struct instruction_desc *desc = &description[cpu->cur_inst[0]];
    if (desc->word == 2) {
        printf("Unimplemented opcode! 0x%x\n", cpu->cur_inst[0]);
        return -1;
    }
    cpu->return_reason = NO_REASON;
    switch (desc->mod) {
    case MOD_REGRM:
        reg = (cpu->cur_inst[1] >> 3) & 7;
        if (desc->word) {
            cpu->operand_reg16 = cpu->reg16 + reg;
        } else {
            cpu->operand_reg8 = cpu->reg8 + REG8INDEX(reg);
        }
        rm = cpu->cur_inst[1] & 7;
        switch (cpu->cur_inst[1] & 0xC0){
        case 0xC0:
            if (desc->word) {
                cpu->operand_rm16 = cpu->reg16 + rm;
            } else {
                cpu->operand_rm8 = cpu->reg8 + REG8INDEX(rm);
            }
            break;
        }
        break;
    case MOD_SEGRM:
        reg = (cpu->cur_inst[1] >> 3) & 3;
        cpu->operand_reg16 = cpu->segreg + reg;
        rm = cpu->cur_inst[1] & 7;
        switch (cpu->cur_inst[1] & 0xC0){
        case 0xC0:
            cpu->operand_rm16 = cpu->reg16 + rm;
            break;
        }
        break;
    case MOD_XXXRM:
        rm = cpu->cur_inst[1] & 7;
        if (desc->word) {
            cpu->operand_rm16 = cpu->reg16 + rm;
        } else {
            cpu->operand_rm8 = cpu->reg8 + REG8INDEX(rm);
        }
        break;
    case REG_REG:
        if (desc->word) {
            cpu->operand_reg16 = cpu->reg16 + (cpu->cur_inst[0] & 7);
        } else {
            cpu->operand_reg8 = cpu->reg8 + REG8INDEX(cpu->cur_inst[0] & 7);
        }
        break;
    case REG_SEG:
        cpu->operand_reg16 = cpu->segreg + ((cpu->cur_inst[0] >> 3) & 7);
        break;
    case MOD_NONE:
        break;
    default:
        printf("Unhandled mod type\n");
        break;
    }
    switch (desc->target) {
    case TARGET_REG:
        if(desc->word) {
            cpu->target16 = cpu->operand_reg16;
        } else {
            cpu->target8 = cpu->operand_reg8;
        }
        break;
    case TARGET_RM:
        if (desc->word) {
            cpu->target16 = cpu->operand_rm16;
        } else {
            cpu->target8 = cpu->operand_rm8;
        }
        break;
    case TARGET_BOTH:
        if (desc->word) {
            cpu->target16 = cpu->operand_reg16;
            cpu->target16_2 = cpu->operand_rm16;
        } else {
            cpu->target8 = cpu->operand_reg8;
            cpu->target8_2 = cpu->operand_rm8;
        }
        break;
    default:
        break;
    }
    return execute(cpu);
    //cpu->next_step = execute;
    //return 0;
}

int fetch(struct iapx88 *cpu)
{
    printf("fetch!\n");
    while (want_more_instruction_bytes(cpu)) {
        if (cpu->prefetch_usage > 0) {
            printf("Yay, prefetched!\n");
            take_instruction_byte_from_prefetch(cpu);
        } else {
            if (cpu->return_reason == WAIT_BIU && cpu->eu_wanted_control_bus_state == BUS_FETCH) {
                cpu->return_reason = NO_REASON;
                take_instruction_byte_from_biu(cpu);
            } else {
                cpu->return_reason = WAIT_BIU;
                cpu->eu_wanted_control_bus_state = BUS_FETCH;
                cpu->eu_wanted_segment = cpu->cs;
                cpu->eu_wanted_offset = cpu->prefetch_ip++;
                return 0;
            }
        }
    }
    cpu->state = CPU_DECODE;
    return decode(cpu);
}

int iapx88_step(struct iapx88 *cpu)
{
    /* uint16_t word1, word2; */
    /* int reg1, reg2; */
    /* uint8_t temp8, modregrm; */

    /* while (1) { */
    /*     switch (cpu->state) { */
    /*     case CPU_FETCH: */
    /*     case CPU_DECODE: */
    /*         if (cpu->cur_inst_len == 0) { */
    /*     	return -1; */
    /*         } */
    /*         switch (cpu->cur_inst[0]) { */
    /*         case 0x32: /\* xor modregrm (to reg8)*\/ */
    /*             modregrm = cpu->cur_inst[1]; */
    /*             reg1 = REG8INDEX((modregrm >> 3) & 7); */
    /*             switch (modregrm & 0xC0) { */
    /*             case 0xC0: */
    /*                 reg2 = REG8INDEX(modregrm & 7); */
    /*                 cpu->reg8[reg1] ^= cpu->reg8[reg2]; */
    /*                 set_flags_from_bitwise8(cpu, cpu->reg8[reg1]); */
    /*                 cleanup(cpu, 0); */
    /*                 return 3; */
    /*             } */
    /*         case 0x33: /\* xor modregrm (to reg16) *\/ */
    /*             modregrm = cpu->cur_inst[1]; */
    /*             reg1 = (modregrm >> 3) & 7; */
    /*             switch (modregrm & 0xC0) { */
    /*             case 0xC0: */
    /*                 reg2 = modregrm & 7; */
    /*                 cpu->reg16[reg1] ^= cpu->reg16[reg2]; */
    /*                 set_flags_from_bitwise16(cpu, cpu->reg16[reg1]); */
    /*                 cleanup(cpu, 0); */
    /*                 return 3; */
    /*             } */
                
    /*         case 0x70: /\* branches *\/ */
    /*             return branch(cpu, cpu->flags & FLAG_OF); // JO */
    /*         case 0x71: */
    /*             return branch(cpu, !(cpu->flags & FLAG_OF)); // JNO */
    /*         case 0x72: */
    /*             return branch(cpu, cpu->flags & FLAG_CF); // JB */
    /*         case 0x73: */
    /*             return branch(cpu, !(cpu->flags & FLAG_CF)); // JAE */
    /*         case 0x74: */
    /*             return branch(cpu, cpu->flags & FLAG_ZF); // JE */
    /*         case 0x75: */
    /*             return branch(cpu, !(cpu->flags & FLAG_ZF)); // JNE */
    /*         case 0x78: */
    /*             return branch(cpu, cpu->flags & FLAG_SF); // JS */
    /*         case 0x79: */
    /*             return branch(cpu, !(cpu->flags & FLAG_SF)); // JNS */
    /*         case 0x7a: */
    /*             iapx88_update_flag_pf(cpu); */
    /*             return branch(cpu, cpu->flags & FLAG_PF); // JP */
    /*         case 0x7b: */
    /*             iapx88_update_flag_pf(cpu); */
    /*             return branch(cpu, !(cpu->flags & FLAG_PF)); // JNP */
    /*         case 0x8b: /\* MOV modregr/m to reg16 *\/ */
    /*             cpu->reg1 = (cpu->cur_inst[1] >> 3) & 7; */
    /*             switch(cpu->cur_inst[1] & 0xC0) { */
    /*             case 0xC0: */
    /*                 cpu->reg2 = cpu->cur_inst[1] & 7; */
    /*                 cpu->reg16[cpu->reg1] = cpu->reg16[cpu->reg2]; */
    /*                 cleanup(cpu, 0); */
    /*                 return 2; */
    /*             } */
    /*         case 0x8c: /\* MOV modregr/m from segreg *\/ */
    /*             cpu->reg1 = (cpu->cur_inst[1] >> 3) & 3; */
    /*             switch (cpu->cur_inst[1] & 0xC0) { */
    /*             case 0xC0: */
    /*                 cpu->reg2 = (cpu->cur_inst[1] & 7); */
    /*                 cpu->reg16[cpu->reg2] = cpu->segreg[cpu->reg1]; */
    /*                 cleanup(cpu, 0); */
    /*                 return 2; */
    /*             } */
    /*         case 0x8e: /\* MOV modregr/m to segreg *\/ */
    /*             cpu->reg1 = (cpu->cur_inst[1] >> 3) & 3; */
    /*             switch (cpu->cur_inst[1] & 0xC0) { */
    /*             case 0xC0: */
    /*                 cpu->reg2 = (cpu->cur_inst[1] & 7); */
    /*                 cpu->segreg[cpu->reg1] = cpu->reg16[cpu->reg2]; */
    /*                 cleanup(cpu, 0); */
    /*                 return 2; */
    /*             } */
    /*         case 0xB0: /\* MOV reg8, immediate *\/ */
    /*         case 0xB1: */
    /*         case 0xB2: */
    /*         case 0xB3: */
    /*         case 0xB4: */
    /*         case 0xB5: */
    /*         case 0xB6: */
    /*         case 0xB7: */
    /*     	cpu->reg1 = REG8INDEX(cpu->cur_inst[0] & 7); */
    /*             cpu->reg8[cpu->reg1] = cpu->cur_inst[1]; */
    /*             cleanup(cpu, 0); */
    /*             return 4; */
    /*         case 0xB8: /\* MOV reg16, immediate *\/ */
    /*         case 0xB9: */
    /*         case 0xBA: */
    /*         case 0xBB: */
    /*         case 0xBC: */
    /*         case 0xBD: */
    /*         case 0xBE: */
    /*         case 0xBF: */
    /*             cpu->reg1 = cpu->cur_inst[0] & 7; */
    /*             cpu->reg16[cpu->reg1] = word_from_bytes(cpu->cur_inst + 1); */
    /*             cleanup(cpu, 0); */
    /*             return 4; */
    /*         case 0x9E: /\* SAHF *\/ */
    /*             cpu->flags = (cpu->flags & 0xFF2A) | (cpu->ah & 0xD5); */
    /*             cpu->flag_pf_source = cpu->flags & FLAG_PF; */
    /*             cleanup(cpu, 0); */
    /*             return 4; */
    /*         case 0x9F: /\* LAHF *\/ */
    /*             iapx88_update_flag_pf(cpu); */
    /*             cpu->ah = (cpu->ah & 0x2A) | (cpu->flags & 0xD5); */
    /*             cleanup(cpu, 0); */
    /*             return 4; */
    /*         case 0xD0: /\* shift/rotate by 1 *\/ */
    /*     	modregrm = cpu->cur_inst[1]; */
    /*     	switch(modregrm & 0x38) { */
    /*     	case 0x20: /\* SHL modxxxrm, 1 *\/ */
    /*     	    switch(modregrm & 0xC0) { */
    /*     	    case 0xC0: */
    /*     		reg1 = REG8INDEX(modregrm & 7); */
    /*     		temp8 = cpu->reg8[reg1] << 1; */
    /*     		set_flag(cpu, FLAG_OF, (cpu->reg8[reg1] & 0x80) ^ (temp8 & 0x80)); */
    /*     		set_flag(cpu, FLAG_CF, (cpu->reg8[reg1] & 0x80)); */
    /*     		cpu->reg8[reg1] = temp8; */
    /*     		cleanup(cpu, 0); */
    /*     		return 2; */
    /*     		break; */
    /*     	    } */
    /*     	    break; */
    /*     	} */
    /*     	break; */
    /*         case 0xD2: /\* shift/rotate by cl *\/ */
    /*     	modregrm = cpu->cur_inst[1]; */
    /*     	switch(cpu->cur_inst[1] & 0x38) { */
    /*     	case 0x28: /\* SHR modxxxrm, cl *\/ */
    /*     	    switch(modregrm & 0xC0) { */
    /*     	    case 0xC0: */
    /*     		reg1 = REG8INDEX(modregrm & 7); */
    /*     		if (cpu->cl > 0) { */
    /*     		    cpu->reg8[reg1] >>= (cpu->cl - 1); */
    /*     		    set_flag(cpu, FLAG_CF, cpu->reg8[reg1] & 1); */
    /*     		    cpu->reg8[reg1] >>=1; */
    /*     		} */
    /*     		cleanup(cpu, 0); */
    /*     		return 8 + 4 * cpu->cl; */
    /*     		break; */
    /*     	    } */
    /*     	    break; */
    /*     	} */
    /*     	break; */
    /*         case 0xEA: /\* JMP direct intersegment *\/ */
    /*     	word1 = word_from_bytes(cpu->cur_inst + 1); */
    /*     	word2 = word_from_bytes(cpu->cur_inst + 3); */
    /*     	cpu->cs = word2; */
    /*     	cpu->ip = word1; */
    /*     	cleanup(cpu, 1); */
    /*     	return 15; */
    /*         case 0xF8: /\* CLC *\/ */
    /*             set_flag(cpu, FLAG_CF, 0); */
    /*             cleanup(cpu, 0); */
    /*             return 2; */
    /*         case 0xF9: /\* STC *\/ */
    /*             set_flag(cpu, FLAG_CF, 1); */
    /*             cleanup(cpu, 0); */
    /*             return 2; */
    /*         case 0xFA: /\* CLI *\/ */
    /*             set_flag(cpu, FLAG_IF, 0); */
    /*     	cleanup(cpu, 0); */
    /*     	return 2; */
    /*         /\* case 0xFF: /\\* PUSH, CALL, JMP, INC, DEC modxxxr/m *\\/ *\/ */
    /*         /\* 	modregrm = cpu->cur_inst[1]; *\/ */
    /*         /\* 	switch (modregrm & 0x38) { *\/ */
    /*         /\* 	case 0x30: *\/ */
    /*         /\* 	    cpu->reg1 = modregrm & 7; *\/ */
    /*         /\* 	} *\/ */
    /*         default: */
    /*     	printf("Unknown opcode: 0x%X\n", cpu->cur_inst[0]); */
    /*     	return -1; */
    /*         } */
    /*     } */
    /* } */
    return 0;
}

/* int run_bus(iapx88 *cpu, int cycles) { */
/*     int c = 0; */
/*     switch (cpu->control_bus_state) { */
/*     case BUS_FETCH: */
/*     case BUS_MEMREAD: */
/* 	switch (cpu->bus_state) { */
/* 	case BUS_T3: */
/* 	    cpu->data_pins = read_memory(cpu->address_pins); // blah */
/* } */

int biu_request_prefetch(struct iapx88 *cpu, int max_cycles)
{
    if (cpu->prefetch_usage == 4) {
	return max_cycles;
    }
    switch (cpu->bus_state) {
    case BUS_IDLE:
	cpu->control_bus_state = BUS_FETCH;
	cpu->bus_state = BUS_T3;
	cpu->address_pins = EA(cpu->cs, cpu->prefetch_ip++);
	return 3;
    case BUS_T1:
	cpu->bus_state = BUS_T3;
	return 2;
    case BUS_T2:
	cpu->bus_state = BUS_T3;
	return 1;
    case BUS_T3:
	return 0;
    case BUS_T4:
	cpu->eu_biu_byte = cpu->data_pins;
	cpu->bus_state = BUS_IDLE;
	cpu->control_bus_state = BUS_NONE;
	return 1;
    case BUS_TW:
	return 1;
    }
}

int biu_handle_prefetch(struct iapx88 *cpu)
{
    int index = (cpu->prefetch_start + cpu->prefetch_usage) & 3;
    cpu->prefetch_queue[index] = cpu->data_pins;
    cpu->prefetch_usage++;
    cpu->bus_state = BUS_IDLE;
    cpu->control_bus_state = BUS_NONE;
    return 1;
}

int biu_make_request(struct iapx88 *cpu)
{
    cpu->control_bus_state = cpu->eu_wanted_control_bus_state;
    cpu->address_pins = EA(cpu->eu_wanted_segment, cpu->eu_wanted_offset);
    if (cpu->control_bus_state == BUS_MEMWRITE) {
	cpu->data_pins = cpu->eu_biu_byte;
    }
    cpu->bus_state = BUS_T3;
    return 3;
}

int biu_handle_response(struct iapx88 *cpu)
{
    if (cpu->control_bus_state == BUS_FETCH || cpu->control_bus_state == BUS_MEMREAD) {
	cpu->eu_biu_byte = cpu->data_pins;
    }
    cpu->control_bus_state = BUS_NONE;
    cpu->bus_state = BUS_IDLE;
    return 1;
}
