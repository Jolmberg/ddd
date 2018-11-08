#ifndef _IAPX88_H_
#define _IAPX88_H_

#include <stdint.h>

enum regs_8 { AL, AH, CL, CH, DL, DH, BL, BH };
enum regs_16 { AX, CX, DX, BX };
enum control_bus_state { BUS_INTA, BUS_IOREAD, BUS_IOWRITE, BUS_HALT, BUS_FETCH, BUS_MEMREAD, BUS_MEMWRITE, BUS_NONE };
enum state { CPU_IDLE, CPU_FETCH, CPU_DECODE, CPU_MEMREAD, CPU_MEMWRITE };
enum bus_state { BUS_IDLE, BUS_T1, BUS_T2, BUS_T3, BUS_T4, BUS_TW };
enum return_reason { NO_REASON, WAIT_BIU, WAIT_INTERRUPTIBLE };

struct iapx88 {
    // Registers
    union {
	uint8_t reg8[8];
	uint16_t reg16[8];
	struct { uint8_t al, ah, cl, ch, dl, dh, bl, bh; };
	struct { uint16_t ax, cx, dx, bx, sp, bp, si, di; };
    };
    union {
	uint16_t segreg[4];
	struct { uint16_t es, cs, ss, ds; };
    };
    uint16_t ip, flags;

    // Pins
    uint32_t address_pins;
    uint8_t data_pins;
    enum control_bus_state control_bus_state;

    // BIU stuff
    uint8_t prefetch_queue[4];
    int prefetch_size;
    int prefetch_offset;
    int prefetch_forbidden;
    enum bus_state bus_state;

    enum return_reason return_reason;
    enum control_bus_state eu_wanted_control_bus_state;
    uint16_t eu_wanted_segment, eu_wanted_offset;
    uint8_t eu_biu_byte;
    
    int state;
    int segment_override;
    uint8_t cur_inst[5];
    int cur_inst_read;
    int cur_inst_len;
    uint16_t prefetch_ip;
    int reg;
};

struct iapx88 *iapx88_create();
void iapx88_reset(struct iapx88 *cpu);
int iapx88_step(struct iapx88 *cpu);

int biu_request_prefetch(struct iapx88 *cpu, int max_cycles);
int biu_handle_prefetch(struct iapx88 *cpu);
int biu_make_request(struct iapx88 *cpu);
int biu_handle_response(struct iapx88 *cpu);

uint32_t ea(uint16_t segment, uint16_t offset);

#endif
