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

enum mod_type { MOD_NONE, MOD_REGRM, MOD_SEGRM, MOD_XXXRM, REG_REG, REG_SEG };
enum rm_rw { RM_READ = 1, RM_WRITE = 2, RM_BOTH = 3};
struct instruction_desc {
    int word;
    enum mod_type mod;
    int rm_rw; // Bit field of rm_rw enums or (for modxxxrm) index into the real rm_rw array.
};


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
    //int eu_wanted_bytes;
    uint16_t eu_wanted_segment, eu_wanted_offset, eu_wanted_port;
    uint8_t eu_biu_byte;
    
    int (*next_step)(struct iapx88 *cpu);
    int state;
    int segment_override;
    uint8_t cur_inst[5];
    int cur_inst_read;
    int cur_inst_len;
    uint16_t prefetch_ip;
    int jumped;
    uint8_t *operand_reg8, *operand_rm8, *target8, *target8_2;
    uint16_t *operand_reg16, *operand_rm16, *target16, *target16_2;
};

struct iapx88 *iapx88_create();
void iapx88_reset(struct iapx88 *cpu);
int iapx88_step(struct iapx88 *cpu);
void iapx88_update_flag_pf(struct iapx88 *cpu);

int biu_request_prefetch(struct iapx88 *cpu, int max_cycles);
int biu_handle_prefetch(struct iapx88 *cpu);
int biu_make_request(struct iapx88 *cpu);
int biu_handle_response(struct iapx88 *cpu);

#define IS_SEGMENT_OVERRIDE(x) (((x) & 0xE7) == 0x26)
#define EA(seg, offs) ((((seg) << 4) + (offs)) & 0xFFFFF)
#define REG8INDEX(reg) ((((reg) << 1) & 7) | ((reg) >> 2))

//int cleanup(struct iapx88 *cpu);

#endif
