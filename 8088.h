#ifndef _IAPX88_H_
#define _IAPX88_H_

#include <stdint.h>

enum regs_8 { AL, AH, CL, CH, DL, DH, BL, BH };
enum regs_16 { AX, CX, DX, BX };
enum control_bus_state { BUS_INTA, BUS_IOREAD, BUS_IOWRITE, BUS_HALT, BUS_FETCH, BUS_MEMREAD, BUS_MEMWRITE, BUS_NONE };
enum state { CPU_IDLE, CPU_FETCH, CPU_DECODE, CPU_MEMREAD, CPU_MEMWRITE };
enum bus_state { BUS_IDLE, BUS_T1, BUS_T2, BUS_T3, BUS_T4, BUS_TW };
enum return_reason { NO_REASON, WAIT_BIU, WAIT_INTERRUPTIBLE };
enum flags { FLAG_OF=0x800,
             FLAG_DF=0x400,
             FLAG_IF=0x200,
             FLAG_TF=0x100,
             FLAG_SF=0x80,
             FLAG_ZF=0x40,
             FLAG_AF=0x10,
             FLAG_PF=0x4,
             FLAG_CF=0x1 };

/* enum instruction_type { _, */
/*                         SIMPLE, */
/*                         REGISTER8_IMMEDIATE, */
/*                         REGISTER16_IMMEDIATE, */
/*                         IMMEDIATE8, */
/*                         IMMEDIATE16, */
/*                         MODREGRM8_TOREG, */
/*                         MODREGRM8_FROMREG, */
/*                         MODREGRM16, */
/*                         MODSEGRM, */
/*                         MODXXXRM8, */
/*                         MODXXXRM16, */
/*                         MODXXXRM8_IMMEDIATE, */
/*                         MODXXXRM16_IMMEDIATE, */
/*                         SILLY, */
/*                         OFFSET_SEGMENT}; */


/* const int (*p)(struct iapx88 *cpu)[32] = { { NULL }, */
/*                                               { do_instruction */
/*                                               {  */


struct iapx88 {
    // Registers
    union {
	uint8_t reg8[8];
	struct { uint8_t al, ah, cl, ch, dl, dh, bl, bh; };
	uint16_t reg16[12];
        struct { uint16_t dummy[8], segreg[4]; };
	struct { uint16_t ax, cx, dx, bx, sp, bp, si, di, es, cs, ss, ds; };
    };
    uint16_t ip, flags;
    uint16_t flag_pf_source; // Calculate parity flag from this when needed

    // Pins
    uint32_t address_pins;
    uint8_t data_pins;
    enum control_bus_state control_bus_state;

    // BIU stuff
    uint8_t prefetch_queue[4];
    int prefetch_usage;
    int prefetch_start;
    int prefetch_forbidden;
    enum bus_state bus_state;

    enum return_reason return_reason;
    enum control_bus_state eu_wanted_control_bus_state;
    uint16_t eu_wanted_segment, eu_wanted_offset;
    uint8_t eu_biu_byte;
    
    int (**plan_step)(struct iapx88 *cpu);
    int (*next_step)(struct iapx88 *cpu);
    int state;
    int segment_override;
    uint8_t cur_inst[5];
    int cur_inst_read;
    int cur_inst_len;
    uint16_t prefetch_ip;
    int reg1, reg2;
    int jumped;
    uint8_t operand8_1, operand8_2;
    uint16_t operand16_1, operand16_2;
};

struct iapx88 *iapx88_create();
void iapx88_reset(struct iapx88 *cpu);
int iapx88_step(struct iapx88 *cpu);
void iapx88_update_flag_pf(struct iapx88 *cpu);

int biu_request_prefetch(struct iapx88 *cpu, int max_cycles);
int biu_handle_prefetch(struct iapx88 *cpu);
int biu_make_request(struct iapx88 *cpu);
int biu_handle_response(struct iapx88 *cpu);

#define IS_SEGMENT_OVERRIDE(x) (((x) & 0xE7) == 0x66)
#define EA(seg, offs) ((((seg) << 4) + (offs)) & 0xFFFFF)
#define REG8INDEX(reg) ((((reg) << 1) & 7) | ((reg) >> 2))

int read_modsegrm16(struct iapx88 *cpu);
int write_modsegrm16(struct iapx88 *cpu);

int read_modsegrm16_to(struct iapx88 *cpu);
int write_modsegrm16_to(struct iapx88 *cpu);

int read_modregrm16_to(struct iapx88 *cpu);
int write_modregrm16_to(struct iapx88 *cpu);

int read_modregrm8(struct iapx88 *cpu);
int write_modregrm8(struct iapx88 *cpu);

int read_modregrm8_to(struct iapx88 *cpu);
int write_modregrm8_to(struct iapx88 *cpu);

int read_modxxxrm8(struct iapx88 *cpu);
int write_modxxxrm8(struct iapx88 *cpu);

int do_operation(struct iapx88 *cpu);
int cleanup(struct iapx88 *cpu);

int xor8(struct iapx88 *cpu);
int xor16(struct iapx88 *cpu);

int mov16(struct iapx88 *cpu);

int jo(struct iapx88 *cpu);
int jno(struct iapx88 *cpu);
int jb(struct iapx88 *cpu);
int jae(struct iapx88 *cpu);
int je(struct iapx88 *cpu);
int jne(struct iapx88 *cpu);
int js(struct iapx88 *cpu);
int jns(struct iapx88 *cpu);
int jp(struct iapx88 *cpu);
int jnp(struct iapx88 *cpu);

int sahf(struct iapx88 *cpu);
int lahf(struct iapx88 *cpu);

int mov_reg8_imm(struct iapx88 *cpu);
int mov_reg16_imm(struct iapx88 *cpu);

int shift8_1(struct iapx88 *cpu);
int shift16_1(struct iapx88 *cpu);
int shift8_cl(struct iapx88 *cpu);
int shift16_cl(struct iapx88 *cpu);

int jmp_direct_is(struct iapx88 *cpu);

int cli(struct iapx88 *cpu);
int clc(struct iapx88 *cpu);
int stc(struct iapx88 *cpu);

#endif
