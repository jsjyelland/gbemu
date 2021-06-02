#include <cpu.h>

#define OPCODE_DEBUG 1

// CPU registers
static uint8_t a, b, c, d, e, f, h, l;

// 16-Bit combined registers
#define AF_R() ((a << 8) | f)
#define AF_W(n) a = ((n) >> 8); f = (n) & 0xFF

#define BC_R() ((b << 8) | c)
#define BC_W(n) b = ((n) >> 8); c = (n) & 0xFF

#define DE_R() ((d << 8) | e)
#define DE_W(n) d = ((n) >> 8); e = (n) & 0xFF

#define HL_R() ((h << 8) | l)
#define HL_W(n) h = ((n) >> 8); l = (n) & 0xFF

// Stack pointer
static uint16_t sp;

// Program counter
static uint16_t pc;

// Cycle counter
static uint32_t cpu_cycles;

// Flag register getters
#define FLAG_Z !!(f & (1 << 7))
#define FLAG_N !!(f & (1 << 6))
#define FLAG_H !!(f & (1 << 5))
#define FLAG_C !!(f & (1 << 4))

// Flag register setters
#define Z_SET() f |= 1 << 7
#define N_SET() f |= 1 << 6
#define H_SET() f |= 1 << 5
#define C_SET() f |= 1 << 4

#define Z_RESET() f &= ~(1 << 7)
#define N_RESET() f &= ~(1 << 6)
#define H_RESET() f &= ~(1 << 5)
#define C_RESET() f &= ~(1 << 4)

#define FLAGS_RESET() f = 0

// Flag helper functions

static void zero_check(uint16_t result) {
    if (!(result & 0xFF)) {
        Z_SET();
    }
}

static void carry_check(uint16_t result) {
    if (result > 0xFF) {
        C_SET();
    }
}

static void half_carry_check(uint8_t a, uint8_t b) {
    if ((a & 0xF) + (b & 0xF) > 0xF) {
        H_SET();
    }
}

// Retreive arguments

/**
 * Retrieve unsigned 8-bit immediate argument
 */
uint8_t uarg8() {
    #if OPCODE_DEBUG
        if (!get_gb_instance()->in_bios) printf("Uarg8: %X\r\n", mem_read_byte(pc));
    #endif
    return mem_read_byte(pc++);
}

/**
 * Retrieve signed 8-bit immediate argument
 */
int8_t arg8() {
    #if OPCODE_DEBUG
        if (!get_gb_instance()->in_bios) printf("Arg8: %X\r\n", mem_read_byte(pc));
    #endif
    return mem_read_byte(pc++);
}

/**
 * Retrieve unsigned 16-bit immediate argument
 */
uint16_t uarg16() {
    #if OPCODE_DEBUG
        if (!get_gb_instance()->in_bios) printf("Arg16: %X\r\n", mem_read_16(pc));
    #endif
    uint16_t arg = mem_read_16(pc);
    pc += 2;
    return arg;
}

/* INSTRUCTIONS */

// 8-Bit loads

/**
 * LD r,n
 * Put value n into r
 */
static void ld_r_n(uint8_t *r, uint8_t n) {
    *(r) = n;
    cpu_cycles += 8;
}

/**
 * LD r1,r2
 * Put value r2 into r1
 */
static void ld_r1_r2(uint8_t *r1, uint8_t *r2) {
    *(r1) = *(r2);
    cpu_cycles += 4;
}

/**
 * LD r,(addr)
 * Put value at addr into r1
 */
static void ld_r_addr(uint8_t *r, uint16_t addr) {
    *(r) = mem_read_16(addr);
    cpu_cycles += 8;
}

/**
 * LD (addr),r
 * Put value in r into addr
 */
static void ld_addr_r(uint8_t *r, uint16_t addr) {
    mem_write_byte(addr, *(r));
    cpu_cycles += 8;
}

/**
 * LD (addr),n
 * Put value n into addr
 */
static void ld_addr_n(uint8_t n, uint16_t addr) {
    mem_write_byte(addr, n);
    cpu_cycles += 12;
}

#define LD_A_A() ld_r1_r2(&a, &a)
#define LD_A_B() ld_r1_r2(&a, &b)
#define LD_A_C() ld_r1_r2(&a, &c)
#define LD_A_D() ld_r1_r2(&a, &d)
#define LD_A_E() ld_r1_r2(&a, &e)
#define LD_A_H() ld_r1_r2(&a, &h)
#define LD_A_L() ld_r1_r2(&a, &l)

#define LD_B_A() ld_r1_r2(&b, &a)
#define LD_B_B() ld_r1_r2(&b, &b)
#define LD_B_C() ld_r1_r2(&b, &c)
#define LD_B_D() ld_r1_r2(&b, &d)
#define LD_B_E() ld_r1_r2(&b, &e)
#define LD_B_H() ld_r1_r2(&b, &h)
#define LD_B_L() ld_r1_r2(&b, &l)

#define LD_C_A() ld_r1_r2(&c, &a)
#define LD_C_B() ld_r1_r2(&c, &b)
#define LD_C_C() ld_r1_r2(&c, &c)
#define LD_C_D() ld_r1_r2(&c, &d)
#define LD_C_E() ld_r1_r2(&c, &e)
#define LD_C_H() ld_r1_r2(&c, &h)
#define LD_C_L() ld_r1_r2(&c, &l)

#define LD_D_A() ld_r1_r2(&d, &a)
#define LD_D_B() ld_r1_r2(&d, &b)
#define LD_D_C() ld_r1_r2(&d, &c)
#define LD_D_D() ld_r1_r2(&d, &d)
#define LD_D_E() ld_r1_r2(&d, &e)
#define LD_D_H() ld_r1_r2(&d, &h)
#define LD_D_L() ld_r1_r2(&d, &l)

#define LD_E_A() ld_r1_r2(&e, &a)
#define LD_E_B() ld_r1_r2(&e, &b)
#define LD_E_C() ld_r1_r2(&e, &c)
#define LD_E_D() ld_r1_r2(&e, &d)
#define LD_E_E() ld_r1_r2(&e, &e)
#define LD_E_H() ld_r1_r2(&e, &h)
#define LD_E_L() ld_r1_r2(&e, &l)

#define LD_H_A() ld_r1_r2(&h, &a)
#define LD_H_B() ld_r1_r2(&h, &b)
#define LD_H_C() ld_r1_r2(&h, &c)
#define LD_H_D() ld_r1_r2(&h, &d)
#define LD_H_E() ld_r1_r2(&h, &e)
#define LD_H_H() ld_r1_r2(&h, &h)
#define LD_H_L() ld_r1_r2(&h, &l)

#define LD_L_A() ld_r1_r2(&l, &a)
#define LD_L_B() ld_r1_r2(&l, &b)
#define LD_L_C() ld_r1_r2(&l, &c)
#define LD_L_D() ld_r1_r2(&l, &d)
#define LD_L_E() ld_r1_r2(&l, &e)
#define LD_L_H() ld_r1_r2(&l, &h)
#define LD_L_L() ld_r1_r2(&l, &l)

#define LD_A_N() ld_r_n(&a, uarg8())
#define LD_B_N() ld_r_n(&b, uarg8())
#define LD_C_N() ld_r_n(&c, uarg8())
#define LD_D_N() ld_r_n(&d, uarg8())
#define LD_E_N() ld_r_n(&e, uarg8())
#define LD_H_N() ld_r_n(&h, uarg8())
#define LD_L_N() ld_r_n(&l, uarg8())

#define LD_A_AHL() ld_r_addr(&a, HL_R())
#define LD_B_AHL() ld_r_addr(&b, HL_R())
#define LD_C_AHL() ld_r_addr(&c, HL_R())
#define LD_D_AHL() ld_r_addr(&d, HL_R())
#define LD_E_AHL() ld_r_addr(&e, HL_R())
#define LD_H_AHL() ld_r_addr(&h, HL_R())
#define LD_L_AHL() ld_r_addr(&l, HL_R())

#define LD_AHL_A() ld_addr_r(&a, HL_R())
#define LD_AHL_B() ld_addr_r(&b, HL_R())
#define LD_AHL_C() ld_addr_r(&c, HL_R())
#define LD_AHL_D() ld_addr_r(&d, HL_R())
#define LD_AHL_E() ld_addr_r(&e, HL_R())
#define LD_AHL_H() ld_addr_r(&h, HL_R())
#define LD_AHL_L() ld_addr_r(&l, HL_R())

#define LD_AHL_N() ld_addr_n(uarg8(), HL_R())

#define LD_A_ABC() ld_r_addr(&a, BC_R())
#define LD_A_ADE() ld_r_addr(&a, DE_R())

#define LD_A_AC() ld_r_addr(&a, 0xFF00 + c)
#define LD_AC_A() ld_addr_r(&a, 0xFF00 + c)

#define LD_A_AN() ld_r_addr(&a, 0xFF00 + uarg8()); cpu_cycles += 4
#define LD_AN_A() ld_addr_r(&a, 0xFF00 + uarg8()); cpu_cycles += 4

#define LD_A_ANN() ld_r_addr(&a, uarg16()); cpu_cycles += 8
#define LD_ANN_A() ld_addr_r(&a, uarg16()); cpu_cycles += 8

#define LD_A_AHLI() LD_A_AHL(); HL_W(HL_R() + 1)
#define LD_A_AHLD() LD_A_AHL(); HL_W(HL_R() - 1)

#define LD_ABC_A() ld_addr_r(&a, BC_R())
#define LD_ADE_A() ld_addr_r(&a, DE_R())

#define LD_AHLI_A() LD_AHL_A(); HL_W(HL_R() + 1)
#define LD_AHLD_A() LD_AHL_A(); HL_W(HL_R() - 1)

// 16-Bit loads

/**
 * LD bc,nn
 * Put value nn into bc
 */
static void ld_rbc_nn(uint16_t nn) {
    BC_W(nn);
    cpu_cycles += 12;
}

/**
 * LD de,nn
 * Put value nn into de
 */
static void ld_rde_nn(uint16_t nn) {
    DE_W(nn);
    cpu_cycles += 12;
}

/**
 * LD hl,nn
 * Put value nn into hl
 */
static void ld_rhl_nn(uint16_t nn) {
    HL_W(nn);
    cpu_cycles += 12;
}

/**
 * LD sp,nn
 * Put value nn into sp
 */
static void ld_rsp_nn(uint16_t nn) {
    sp = nn;
    cpu_cycles += 12;
}

/**
 * LD sp,hl
 * Put value of hl into sp
 */
static void ld_rsp_rhl() {
    sp = HL_R();
    cpu_cycles += 8;
}

/**
 * PUSH qq
 * Push value in register pair qq onto the stack.
 * r1 should be the higher byte in the pair.
 * Decrement sp twice
 */
static void push_nn(uint8_t *r1, uint8_t *r2) {
    sp--;
    mem_write_byte(sp, *(r1));
    sp--;
    mem_write_byte(sp, *(r2));
    cpu_cycles += 16;
}

/**
 * POP qq
 * Pop two bytes off the stack into register pair qq.
 * r1 should be the higher byte in the pair.
 * Increment sp twice
 */
static void pop_nn(uint8_t *r1, uint8_t *r2) {
    *(r2) = mem_read_byte(sp);
    sp++;
    *(r1) = mem_read_byte(sp);
    sp++;
    cpu_cycles += 12;
}

/**
 * Load sp + signed byte operand n in hl
 */
static void ldhl_n(int8_t n) {
    FLAGS_RESET();

    uint16_t result = sp + n;

    // Flags
    if (n >= 0) {
        if ((sp & 0xFF) + n > 0xFF) {
            C_SET();
        }

        if ((sp & 0xF) + n > 0xF) {
            H_SET();
        }
    } else {
        if ((result & 0xFF) <= (sp & 0xFF)) {
            C_SET();
        }

        if ((result & 0xF) <= (sp & 0xF)) {
            H_SET();
        }
    }
    
    HL_W(result);
    cpu_cycles += 12;
}

/**
 * Load sp into memory at address (nn)
 */
static void ld_ann_sp(uint16_t nn) {
    mem_write_16(nn, sp);
    cpu_cycles += 20;
}

#define LD_BC_NN() ld_rbc_nn(uarg16())
#define LD_DE_NN() ld_rde_nn(uarg16())
#define LD_HL_NN() ld_rhl_nn(uarg16())
#define LD_SP_NN() ld_rsp_nn(uarg16())

#define LD_SP_HL() ld_rsp_rhl()

#define PUSH_BC() push_nn(&b, &c)
#define PUSH_DE() push_nn(&d, &e)
#define PUSH_HL() push_nn(&h, &l)
#define PUSH_AF() push_nn(&a, &f)

#define POP_AF() pop_nn(&a, &f)
#define POP_BC() pop_nn(&b, &c)
#define POP_DE() pop_nn(&d, &e)
#define POP_HL() pop_nn(&h, &l)

#define LDHL_SP_E() ldhl_n(arg8())
#define LD_ANN_SP() ld_ann_sp(uarg16())

// 8-Bit ALU

/**
 * Add byte operand n to a and store in a
 */
static void add_a_n(uint8_t n) {
    FLAGS_RESET();
    uint16_t result = a + n;

    carry_check(result);
    half_carry_check(a, n);
    zero_check(result);

    a = result & 0xFF;

    cpu_cycles += 4;
}

/**
 * Add contents of r to a and store in a
 */
static void add_a_r(uint8_t *r) {
    add_a_n(*(r));
}

/**
 * Add contents of memory at addr to a and store in a
 */
static void add_a_addr(uint16_t addr) {
    add_a_n(mem_read_byte(addr));
    cpu_cycles += 4;
}

/**
 * Add the contents of r to a and store in a,
 * adding c
 */
static void adc_a_r(uint8_t *r) {
    add_a_n(*(r) + FLAG_C);
}

/**
 * Add the contents of memory at addr to a and store in a,
 * adding c
 */
static void adc_a_addr(uint16_t addr) {
    add_a_n(mem_read_byte(addr) + FLAG_C);
    cpu_cycles += 4;
}

/**
 * Add n to a and store in a,
 * adding c
 */
static void adc_a_n(uint8_t n) {
    add_a_n(n + FLAG_C);
    cpu_cycles += 4;
}

/**
 * Subtract n from a and store the result
 * in a.
 */
static void sub_a_n(uint8_t n) {
    FLAGS_RESET();
    N_SET();

    uint8_t result = a - n;

    // Flags
    if (a < n) {
        C_SET();
    }

    if ((a & 0xF) < (n & 0xF)) {
        H_SET();
    }

    zero_check(result);

    a = result;
    cpu_cycles += 4;
}

/**
 * Subtract the value in r from a and store
 * the result in a.
 */
static void sub_a_r(uint8_t *r) {
    sub_a_n(*(r));
}

/**
 * Subtract the value in memory at hl from a
 * and store the result in a.
 */
static void sub_a_addr(uint16_t addr) {
    sub_a_n(mem_read_byte(HL_R()));
    cpu_cycles += 4;
}

/**
 * Subtract the contents of r from a and store in a,
 * subtracting c
 */
static void sbc_a_r(uint8_t *r) {
    sub_a_n(*(r) - FLAG_C);
}

/**
 * Subtract the contents of memory at addr from a and store in a,
 * subtracting c
 */
static void sbc_a_addr(uint16_t addr) {
    sub_a_n(mem_read_byte(addr) - FLAG_C);
    cpu_cycles += 4;
}

/**
 * Subtract n from a and store in a,
 * subtracting c
 */
static void sbc_a_n(uint8_t n) {
    sub_a_n(n - FLAG_C);
    cpu_cycles += 4;
}

/**
 * And a with n, storing the result in a
 */
static void and_a_n(uint8_t n) {
    FLAGS_RESET();
    H_SET();
    a = a & n;
    zero_check(a);
    cpu_cycles += 4;
}

/**
 * And a with value in r, storing the result in a
 */
static void and_a_r(uint8_t *r) {
    and_a_n(*(r));
}

/**
 * And a with the value at memory at addr,
 * storing the result in a
 */
static void and_a_addr(uint16_t addr) {
    and_a_n(mem_read_byte(addr));
    cpu_cycles += 4;
}

/**
 * Or a with n, storing the result in a
 */
static void or_a_n(uint8_t n) {
    FLAGS_RESET();
    a = a | n;
    zero_check(a);
    cpu_cycles += 4;
}

/**
 * Or a with value in r, storing the result in a
 */
static void or_a_r(uint8_t *r) {
    or_a_n(*(r));
}

/**
 * Or a with the value at memory at addr,
 * storing the result in a
 */
static void or_a_addr(uint16_t addr) {
    or_a_n(mem_read_byte(addr));
    cpu_cycles += 4;
}

/**
 * Xor a with n, storing the result in a
 */
static void xor_a_n(uint8_t n) {
    FLAGS_RESET();
    a = a ^ n;
    zero_check(a);
    cpu_cycles += 4;
}

/**
 * Xor a with value in r, storing the result in a
 */
static void xor_a_r(uint8_t *r) {
    xor_a_n(*(r));
}

/**
 * Xor a with the value at memory at addr,
 * storing the result in a
 */
static void xor_a_addr(uint16_t addr) {
    xor_a_n(mem_read_byte(addr));
    cpu_cycles += 4;
}

/**
 * Compare a with n, storing the result in a
 */
static void cp_a_n(uint8_t n) {
    FLAGS_RESET();
    N_SET();

    uint8_t result = a - n;

    // Flags
    if (a < n) {
        C_SET();
    }

    if ((a & 0xF) < (n & 0xF)) {
        H_SET();
    }

    zero_check(result);

    cpu_cycles += 4;
}

/**
 * Compare a with value in r, storing the result in a
 */
static void cp_a_r(uint8_t *r) {
    cp_a_n(*(r));
}

/**
 * Compare a with the value at memory at addr,
 * storing the result in a
 */
static void cp_a_addr(uint16_t addr) {
    cp_a_n(mem_read_byte(addr));
    cpu_cycles += 4;
}

/**
 * Increment register r by 1
 */
static void inc_r(uint8_t *r) {
    H_RESET();
    N_RESET();
    Z_RESET();
    
    uint16_t result = *(r) + 1;

    half_carry_check(*(r), 1);
    zero_check(result);

    *(r) = result & 0xFF;

    cpu_cycles += 4;
}

/**
 * Increment the value at (hl) by 1
 */
static void inc_ahl() {
    H_RESET();
    N_RESET();
    Z_RESET();

    uint8_t val = mem_read_byte(HL_R());
    uint16_t result = val + 1;
    half_carry_check(val, 1);
    zero_check(result);
    mem_write_byte(HL_R(), result & 0xFF);

    cpu_cycles += 12;
}

/**
 * Decrement register r by 1
 */
static void dec_r(uint8_t *r) {
    H_RESET();
    N_SET();
    Z_RESET();
    
    uint8_t result = *(r) - 1;

    if (*(r) < 1) {
        H_SET();
    }

    zero_check(result);

    *(r) = result;

    cpu_cycles += 4;
}

/**
 * Decrement the value at (hl) by 1
 */
static void dec_ahl() {
    H_RESET();
    N_RESET();
    Z_RESET();

    uint8_t val = mem_read_byte(HL_R());
    uint8_t result = val - 1;
   
    if (val < 1) {
        H_SET();
    }
    
    zero_check(result);
    mem_write_byte(HL_R(), result);

    cpu_cycles += 12;
}

#define ADD_A_A() add_a_r(&a)
#define ADD_A_B() add_a_r(&b)
#define ADD_A_C() add_a_r(&c)
#define ADD_A_D() add_a_r(&d)
#define ADD_A_E() add_a_r(&e)
#define ADD_A_H() add_a_r(&h)
#define ADD_A_L() add_a_r(&l)
#define ADD_A_N() add_a_n(uarg8()); cpu_cycles += 4
#define ADD_A_AHL() add_a_addr(HL_R())

#define ADC_A_A() adc_a_r(&a)
#define ADC_A_B() adc_a_r(&b)
#define ADC_A_C() adc_a_r(&c)
#define ADC_A_D() adc_a_r(&d)
#define ADC_A_E() adc_a_r(&e)
#define ADC_A_H() adc_a_r(&h)
#define ADC_A_L() adc_a_r(&l)
#define ADC_A_N() adc_a_n(uarg8())
#define ADC_A_AHL() adc_a_addr(HL_R())

#define SUB_A_A() sub_a_r(&a)
#define SUB_A_B() sub_a_r(&b)
#define SUB_A_C() sub_a_r(&c)
#define SUB_A_D() sub_a_r(&d)
#define SUB_A_E() sub_a_r(&e)
#define SUB_A_H() sub_a_r(&h)
#define SUB_A_L() sub_a_r(&l)
#define SUB_A_N() sub_a_n(uarg8()); cpu_cycles += 4
#define SUB_A_AHL() sub_a_addr(HL_R())

#define SBC_A_A() sbc_a_r(&a)
#define SBC_A_B() sbc_a_r(&b)
#define SBC_A_C() sbc_a_r(&c)
#define SBC_A_D() sbc_a_r(&d)
#define SBC_A_E() sbc_a_r(&e)
#define SBC_A_H() sbc_a_r(&h)
#define SBC_A_L() sbc_a_r(&l)
#define SBC_A_N() sbc_a_n(uarg8())
#define SBC_A_AHL() sbc_a_addr(HL_R())

#define AND_A_A() and_a_r(&a)
#define AND_A_B() and_a_r(&b)
#define AND_A_C() and_a_r(&c)
#define AND_A_D() and_a_r(&d)
#define AND_A_E() and_a_r(&e)
#define AND_A_H() and_a_r(&h)
#define AND_A_L() and_a_r(&l)
#define AND_A_N() and_a_n(uarg8()); cpu_cycles += 4
#define AND_A_AHL() and_a_addr(HL_R())

#define OR_A_A() or_a_r(&a)
#define OR_A_B() or_a_r(&b)
#define OR_A_C() or_a_r(&c)
#define OR_A_D() or_a_r(&d)
#define OR_A_E() or_a_r(&e)
#define OR_A_H() or_a_r(&h)
#define OR_A_L() or_a_r(&l)
#define OR_A_N() or_a_n(uarg8()); cpu_cycles += 4
#define OR_A_AHL() or_a_addr(HL_R())

#define XOR_A_A() xor_a_r(&a)
#define XOR_A_B() xor_a_r(&b)
#define XOR_A_C() xor_a_r(&c)
#define XOR_A_D() xor_a_r(&d)
#define XOR_A_E() xor_a_r(&e)
#define XOR_A_H() xor_a_r(&h)
#define XOR_A_L() xor_a_r(&l)
#define XOR_A_N() xor_a_n(uarg8()); cpu_cycles += 4
#define XOR_A_AHL() xor_a_addr(HL_R())

#define CP_A_A() cp_a_r(&a)
#define CP_A_B() cp_a_r(&b)
#define CP_A_C() cp_a_r(&c)
#define CP_A_D() cp_a_r(&d)
#define CP_A_E() cp_a_r(&e)
#define CP_A_H() cp_a_r(&h)
#define CP_A_L() cp_a_r(&l)
#define CP_A_N() cp_a_n(uarg8()); cpu_cycles += 4
#define CP_A_AHL() cp_a_addr(HL_R())

#define INC_A() inc_r(&a)
#define INC_B() inc_r(&b)
#define INC_C() inc_r(&c)
#define INC_D() inc_r(&d)
#define INC_E() inc_r(&e)
#define INC_H() inc_r(&h)
#define INC_L() inc_r(&l)

#define INC_AHL() inc_ahl()

#define DEC_A() dec_r(&a)
#define DEC_B() dec_r(&b)
#define DEC_C() dec_r(&c)
#define DEC_D() dec_r(&d)
#define DEC_E() dec_r(&e)
#define DEC_H() dec_r(&h)
#define DEC_L() dec_r(&l)

#define DEC_AHL() dec_ahl()

// 16-Bit ALU

/**
 * Add contents of register ss to hl and store result in hl
 */
static void add_hl_ss(uint16_t ss) {
    C_RESET();
    H_RESET();
    N_RESET();

    uint32_t result = HL_R() + ss;

    if ((HL_R() & 0xFFF) + (ss & 0xFFF) > 0xFFF) {
        H_SET();
    }

    if (result > 0xFFFF) {
        C_SET();
    }

    HL_W(result & 0xFFFF);

    cpu_cycles += 8;
}

/**
 * Add contents of e to sp and store result in sp
 */
static void add_sp_e(uint8_t e) {
    FLAGS_RESET();

    uint32_t result = sp + e;

    if ((sp & 0xFFF) + e > 0xFFF) {
        H_SET();
    }

    if (result > 0xFFFF) {
        C_SET();
    }

    sp = result & 0xFFFF;

    cpu_cycles += 16;
}

/**
 * Increment register bc by 1
 */
static void inc_bc() {
    BC_W((BC_R() + 1) & 0xFFFF);
    cpu_cycles += 8;
}

/**
 * Increment register de by 1
 */
static void inc_de() {
    DE_W((DE_R() + 1) & 0xFFFF);
    cpu_cycles += 8;
}

/**
 * Increment register hl by 1
 */
static void inc_hl() {
    HL_W((HL_R() + 1) & 0xFFFF);
    cpu_cycles += 8;
}

/**
 * Increment register sp by 1
 */
static void inc_sp() {
    sp++;
    cpu_cycles += 8;
}

/**
 * Decrement register bc by 1
 */
static void dec_bc() {
    BC_W((BC_R() - 1) & 0xFFFF);
    cpu_cycles += 8;
}

/**
 * Decrement register de by 1
 */
static void dec_de() {
    DE_W((DE_R() - 1) & 0xFFFF);
    cpu_cycles += 8;
}

/**
 * Decrement register hl by 1
 */
static void dec_hl() {
    HL_W((HL_R() - 1) & 0xFFFF);
    cpu_cycles += 8;
}

/**
 * Decrement register sp by 1
 */
static void dec_sp() {
    sp--;
    cpu_cycles += 8;
}

#define ADD_HL_BC() add_hl_ss(BC_R())
#define ADD_HL_DE() add_hl_ss(DE_R())
#define ADD_HL_HL() add_hl_ss(HL_R())
#define ADD_HL_SP() add_hl_ss(sp)

#define ADD_SP_E() add_sp_e(uarg8())

#define INC_BC() inc_bc()
#define INC_DE() inc_de()
#define INC_HL() inc_hl()
#define INC_SP() inc_sp()

#define DEC_BC() dec_bc()
#define DEC_DE() dec_de()
#define DEC_HL() dec_hl()
#define DEC_SP() dec_sp()

// Rotate shift

/**
 * Rotate r to the left, leaving the carried bit in the c
 * flag and in bit 0
 */
static void rlcr(uint8_t *r) {
    FLAGS_RESET();
    uint16_t result = *(r) << 1;

    if (!!(result & 0x100)) {
        C_SET();
        result |= 0x1;
    }

    *(r) = result & 0xFF;

    zero_check(*(r));

    cpu_cycles += 4;
}

/**
 * Rotate r to the left, leaving the carried bit in the c flag
 * and what was in the c flag in bit 0.
 */
static void rlr(uint8_t *r) {
    uint8_t carry = FLAG_C;
    uint16_t result = *(r) << 1;

    FLAGS_RESET();

    if (!!(result & 0x100)) {
        C_SET();
    }

    *(r) = (result & 0xFF) | carry;

    zero_check(*(r));

    cpu_cycles += 4;
}

/**
 * Rotate r to the right, leaving the carried bit in the c
 * flag and in bit 7
 */
static void rrcr(uint8_t *r) {
    FLAGS_RESET();
    uint8_t bit_zero = *(r) & 0x1;
    uint8_t result = *(r) >> 1;

    if (bit_zero) {
        C_SET();
        result |= 0x80;
    }

    *(r) = result;

    zero_check(*(r));

    cpu_cycles += 4;
}

/**
 * Rotate r to the right, leaving the carried bit in the c flag
 * and what was in the c flag in bit 7.
 */
static void rrr(uint8_t *r) {
    uint8_t bit_zero = *(r) & 0x1;
    uint8_t carry = FLAG_C;

    FLAGS_RESET();

    uint8_t result = *(r) >> 1;

    if (bit_zero) {
        C_SET();
    }

    *(r) = result | (carry << 7);

    zero_check(*(r));

    cpu_cycles += 4;
}

/**
 * RLC with the value stored in (hl)
 */
static void rlc_ahl() {
    uint8_t val = mem_read_byte(HL_R());
    rlcr(&val);
    mem_write_byte(HL_R(), val);

    cpu_cycles += 12;
}

/**
 * RL with the value stored in (hl)
 */
static void rl_ahl() {
    uint8_t val = mem_read_byte(HL_R());
    rlcr(&val);
    mem_write_byte(HL_R(), val);

    cpu_cycles += 12;
}

/**
 * RRC with the value stored in (hl)
 */
static void rrc_ahl() {
    uint8_t val = mem_read_byte(HL_R());
    rlcr(&val);
    mem_write_byte(HL_R(), val);

    cpu_cycles += 12;
}

/**
 * RR with the value stored in (hl)
 */
static void rr_ahl() {
    uint8_t val = mem_read_byte(HL_R());
    rlcr(&val);
    mem_write_byte(HL_R(), val);

    cpu_cycles += 12;
}

/**
 * Shift register r left into carry. LSB set to 0
 */
static void sla_r(uint8_t *r) {
    FLAGS_RESET();

    if (!!(*(r) & 0x80)) {
        C_SET();
    }

    *(r) = (*(r) << 1) & 0xFF;

    zero_check(*(r));

    cpu_cycles += 8;
}

/**
 * Shift value at hl left into carry. LSB set to 0
 */
static void sla_ahl() {
    uint8_t val = mem_read_byte(HL_R());
    sla_r(&val);
    mem_write_byte(HL_R(), val);
    cpu_cycles += 8;
}

/**
 * Shift register r right into carry. MSB not changed
 */
static void sra_r(uint8_t *r) {
    FLAGS_RESET();

    if (!!(*(r) & 0x1)) {
        C_SET();
    }

    *(r) = *(r) >> 1 | (*(r) & 0x80);

    zero_check(*(r));

    cpu_cycles += 8;
}

/**
 * Shift value at hl right into carry. MSB not changed
 */
static void sra_ahl() {
    uint8_t val = mem_read_byte(HL_R());
    sra_r(&val);
    mem_write_byte(HL_R(), val);
    cpu_cycles += 8;
}

/**
 * Shift register r right into carry. MSB set to 0
 */
static void srl_r(uint8_t *r) {
    FLAGS_RESET();

    if (!!(*(r) & 0x1)) {
        C_SET();
    }

    *(r) = *(r) >> 1;

    zero_check(*(r));

    cpu_cycles += 8;
}

/**
 * Shift value at hl right into carry. MSB set to 0
 */
static void srl_ahl() {
    uint8_t val = mem_read_byte(HL_R());
    srl_r(&val);
    mem_write_byte(HL_R(), val);
    cpu_cycles += 8;
}

/**
 * Swap the two nibbles of r
 */
static void swap(uint8_t *r) {
    *(r) = ((*(r) & 0xF) << 4) | ((*(r) & 0xF0) >> 4);
    cpu_cycles += 8;
}

/**
 * Swap the two nibbles of memory (hl)
 */
static void swap_ahl() {
    uint8_t val = mem_read_byte(HL_R());
    swap(&val);
    mem_write_byte(HL_R(), val);
    cpu_cycles += 8;
}

#define RLCA() rlcr(&a)
#define RLA() rlr(&a)
#define RRCA() rrcr(&a)
#define RRA() rrr(&a)

#define RLC_A() rlcr(&a); cpu_cycles += 4
#define RLC_B() rlcr(&b); cpu_cycles += 4
#define RLC_C() rlcr(&c); cpu_cycles += 4
#define RLC_D() rlcr(&d); cpu_cycles += 4
#define RLC_E() rlcr(&e); cpu_cycles += 4
#define RLC_H() rlcr(&h); cpu_cycles += 4
#define RLC_L() rlcr(&l); cpu_cycles += 4

#define RL_A() rlr(&a); cpu_cycles += 4
#define RL_B() rlr(&b); cpu_cycles += 4
#define RL_C() rlr(&c); cpu_cycles += 4
#define RL_D() rlr(&d); cpu_cycles += 4
#define RL_E() rlr(&e); cpu_cycles += 4
#define RL_H() rlr(&h); cpu_cycles += 4
#define RL_L() rlr(&l); cpu_cycles += 4

#define RRC_A() rrcr(&a); cpu_cycles += 4
#define RRC_B() rrcr(&b); cpu_cycles += 4
#define RRC_C() rrcr(&c); cpu_cycles += 4
#define RRC_D() rrcr(&d); cpu_cycles += 4
#define RRC_E() rrcr(&e); cpu_cycles += 4
#define RRC_H() rrcr(&h); cpu_cycles += 4
#define RRC_L() rrcr(&l); cpu_cycles += 4

#define RR_A() rrr(&a); cpu_cycles += 4
#define RR_B() rrr(&b); cpu_cycles += 4
#define RR_C() rrr(&c); cpu_cycles += 4
#define RR_D() rrr(&d); cpu_cycles += 4
#define RR_E() rrr(&e); cpu_cycles += 4
#define RR_H() rrr(&h); cpu_cycles += 4
#define RR_L() rrr(&l); cpu_cycles += 4

#define RLC_AHL() rlc_ahl()
#define RL_AHL() rl_ahl()
#define RRC_AHL() rrc_ahl()
#define RR_AHL() rr_ahl()

#define SLA_A() sla_r(&a); cpu_cycles += 4
#define SLA_B() sla_r(&b); cpu_cycles += 4
#define SLA_C() sla_r(&c); cpu_cycles += 4
#define SLA_D() sla_r(&d); cpu_cycles += 4
#define SLA_E() sla_r(&e); cpu_cycles += 4
#define SLA_H() sla_r(&h); cpu_cycles += 4
#define SLA_L() sla_r(&l); cpu_cycles += 4

#define SRA_A() sra_r(&a); cpu_cycles += 4
#define SRA_B() sra_r(&b); cpu_cycles += 4
#define SRA_C() sra_r(&c); cpu_cycles += 4
#define SRA_D() sra_r(&d); cpu_cycles += 4
#define SRA_E() sra_r(&e); cpu_cycles += 4
#define SRA_H() sra_r(&h); cpu_cycles += 4
#define SRA_L() sra_r(&l); cpu_cycles += 4

#define SRL_A() srl_r(&a); cpu_cycles += 4
#define SRL_B() srl_r(&b); cpu_cycles += 4
#define SRL_C() srl_r(&c); cpu_cycles += 4
#define SRL_D() srl_r(&d); cpu_cycles += 4
#define SRL_E() srl_r(&e); cpu_cycles += 4
#define SRL_H() srl_r(&h); cpu_cycles += 4
#define SRL_L() srl_r(&l); cpu_cycles += 4

#define SLA_AHL() sla_hl()
#define SRA_AHL() sra_hl()
#define SRL_AHL() srl_hl()

#define SWAP_A() swap(&a);
#define SWAP_B() swap(&b);
#define SWAP_C() swap(&c);
#define SWAP_D() swap(&d);
#define SWAP_E() swap(&e);
#define SWAP_H() swap(&h);
#define SWAP_L() swap(&l);
#define SWAP_AHL() swap_ahl();

// Bit operations

/**
 * Copies the compliment of the contents of bit b in r to z flag
 */
static void bit(uint8_t b, uint8_t *r) {
    H_SET();
    N_RESET();

    Z_RESET();

    if (!(*(r) & (1 << b))) {
        Z_SET();
    }

    cpu_cycles += 8;
}

/**
 * Copies the compliment of the contents of bit b in (hl) to z flag
 */
static void bit_ahl(uint8_t b) {
    uint8_t val = mem_read_byte(HL_R());
    bit(b, &val);

    cpu_cycles += 8;
}

/**
 * Set bit b to 1 in r
 */
static void set(uint8_t b, uint8_t *r) {
    *(r) = *(r) | (1 << b);
    cpu_cycles += 8;
}

/**
 * Set bit b to 1 in (hl)
 */
static void set_ahl(uint8_t b) {
    uint8_t val = mem_read_byte(HL_R());
    set(b, &val);

    cpu_cycles += 8;
}

/**
 * Reset bit b to 0 in r
 */
static void res(uint8_t b, uint8_t *r) {
    *(r) = *(r) & ~(1 << b);
    cpu_cycles += 8;
}

/**
 * Reset bit b to 0 in (hl)
 */
static void res_ahl(uint8_t b) {
    uint8_t val = mem_read_byte(HL_R());
    res(b, &val);

    cpu_cycles += 8;
}

#define BIT_0_A() bit(0, &a)
#define BIT_1_A() bit(1, &a)
#define BIT_2_A() bit(2, &a)
#define BIT_3_A() bit(3, &a)
#define BIT_4_A() bit(4, &a)
#define BIT_5_A() bit(5, &a)
#define BIT_6_A() bit(6, &a)
#define BIT_7_A() bit(7, &a)
#define BIT_0_B() bit(0, &b)
#define BIT_1_B() bit(1, &b)
#define BIT_2_B() bit(2, &b)
#define BIT_3_B() bit(3, &b)
#define BIT_4_B() bit(4, &b)
#define BIT_5_B() bit(5, &b)
#define BIT_6_B() bit(6, &b)
#define BIT_7_B() bit(7, &b)
#define BIT_0_C() bit(0, &c)
#define BIT_1_C() bit(1, &c)
#define BIT_2_C() bit(2, &c)
#define BIT_3_C() bit(3, &c)
#define BIT_4_C() bit(4, &c)
#define BIT_5_C() bit(5, &c)
#define BIT_6_C() bit(6, &c)
#define BIT_7_C() bit(7, &c)
#define BIT_0_D() bit(0, &d)
#define BIT_1_D() bit(1, &d)
#define BIT_2_D() bit(2, &d)
#define BIT_3_D() bit(3, &d)
#define BIT_4_D() bit(4, &d)
#define BIT_5_D() bit(5, &d)
#define BIT_6_D() bit(6, &d)
#define BIT_7_D() bit(7, &d)
#define BIT_0_E() bit(0, &e)
#define BIT_1_E() bit(1, &e)
#define BIT_2_E() bit(2, &e)
#define BIT_3_E() bit(3, &e)
#define BIT_4_E() bit(4, &e)
#define BIT_5_E() bit(5, &e)
#define BIT_6_E() bit(6, &e)
#define BIT_7_E() bit(7, &e)
#define BIT_0_H() bit(0, &h)
#define BIT_1_H() bit(1, &h)
#define BIT_2_H() bit(2, &h)
#define BIT_3_H() bit(3, &h)
#define BIT_4_H() bit(4, &h)
#define BIT_5_H() bit(5, &h)
#define BIT_6_H() bit(6, &h)
#define BIT_7_H() bit(7, &h)
#define BIT_0_L() bit(0, &l)
#define BIT_1_L() bit(1, &l)
#define BIT_2_L() bit(2, &l)
#define BIT_3_L() bit(3, &l)
#define BIT_4_L() bit(4, &l)
#define BIT_5_L() bit(5, &l)
#define BIT_6_L() bit(6, &l)
#define BIT_7_L() bit(7, &l)
#define BIT_0_AHL() bit_ahl(0)
#define BIT_1_AHL() bit_ahl(1)
#define BIT_2_AHL() bit_ahl(2)
#define BIT_3_AHL() bit_ahl(3)
#define BIT_4_AHL() bit_ahl(4)
#define BIT_5_AHL() bit_ahl(5)
#define BIT_6_AHL() bit_ahl(6)
#define BIT_7_AHL() bit_ahl(7)

#define SET_0_A() set(0, &a)
#define SET_1_A() set(1, &a)
#define SET_2_A() set(2, &a)
#define SET_3_A() set(3, &a)
#define SET_4_A() set(4, &a)
#define SET_5_A() set(5, &a)
#define SET_6_A() set(6, &a)
#define SET_7_A() set(7, &a)
#define SET_0_B() set(0, &b)
#define SET_1_B() set(1, &b)
#define SET_2_B() set(2, &b)
#define SET_3_B() set(3, &b)
#define SET_4_B() set(4, &b)
#define SET_5_B() set(5, &b)
#define SET_6_B() set(6, &b)
#define SET_7_B() set(7, &b)
#define SET_0_C() set(0, &c)
#define SET_1_C() set(1, &c)
#define SET_2_C() set(2, &c)
#define SET_3_C() set(3, &c)
#define SET_4_C() set(4, &c)
#define SET_5_C() set(5, &c)
#define SET_6_C() set(6, &c)
#define SET_7_C() set(7, &c)
#define SET_0_D() set(0, &d)
#define SET_1_D() set(1, &d)
#define SET_2_D() set(2, &d)
#define SET_3_D() set(3, &d)
#define SET_4_D() set(4, &d)
#define SET_5_D() set(5, &d)
#define SET_6_D() set(6, &d)
#define SET_7_D() set(7, &d)
#define SET_0_E() set(0, &e)
#define SET_1_E() set(1, &e)
#define SET_2_E() set(2, &e)
#define SET_3_E() set(3, &e)
#define SET_4_E() set(4, &e)
#define SET_5_E() set(5, &e)
#define SET_6_E() set(6, &e)
#define SET_7_E() set(7, &e)
#define SET_0_H() set(0, &h)
#define SET_1_H() set(1, &h)
#define SET_2_H() set(2, &h)
#define SET_3_H() set(3, &h)
#define SET_4_H() set(4, &h)
#define SET_5_H() set(5, &h)
#define SET_6_H() set(6, &h)
#define SET_7_H() set(7, &h)
#define SET_0_L() set(0, &l)
#define SET_1_L() set(1, &l)
#define SET_2_L() set(2, &l)
#define SET_3_L() set(3, &l)
#define SET_4_L() set(4, &l)
#define SET_5_L() set(5, &l)
#define SET_6_L() set(6, &l)
#define SET_7_L() set(7, &l)
#define SET_0_AHL() set_ahl(0)
#define SET_1_AHL() set_ahl(1)
#define SET_2_AHL() set_ahl(2)
#define SET_3_AHL() set_ahl(3)
#define SET_4_AHL() set_ahl(4)
#define SET_5_AHL() set_ahl(5)
#define SET_6_AHL() set_ahl(6)
#define SET_7_AHL() set_ahl(7)

#define RES_0_A() res(0, &a)
#define RES_1_A() res(1, &a)
#define RES_2_A() res(2, &a)
#define RES_3_A() res(3, &a)
#define RES_4_A() res(4, &a)
#define RES_5_A() res(5, &a)
#define RES_6_A() res(6, &a)
#define RES_7_A() res(7, &a)
#define RES_0_B() res(0, &b)
#define RES_1_B() res(1, &b)
#define RES_2_B() res(2, &b)
#define RES_3_B() res(3, &b)
#define RES_4_B() res(4, &b)
#define RES_5_B() res(5, &b)
#define RES_6_B() res(6, &b)
#define RES_7_B() res(7, &b)
#define RES_0_C() res(0, &c)
#define RES_1_C() res(1, &c)
#define RES_2_C() res(2, &c)
#define RES_3_C() res(3, &c)
#define RES_4_C() res(4, &c)
#define RES_5_C() res(5, &c)
#define RES_6_C() res(6, &c)
#define RES_7_C() res(7, &c)
#define RES_0_D() res(0, &d)
#define RES_1_D() res(1, &d)
#define RES_2_D() res(2, &d)
#define RES_3_D() res(3, &d)
#define RES_4_D() res(4, &d)
#define RES_5_D() res(5, &d)
#define RES_6_D() res(6, &d)
#define RES_7_D() res(7, &d)
#define RES_0_E() res(0, &e)
#define RES_1_E() res(1, &e)
#define RES_2_E() res(2, &e)
#define RES_3_E() res(3, &e)
#define RES_4_E() res(4, &e)
#define RES_5_E() res(5, &e)
#define RES_6_E() res(6, &e)
#define RES_7_E() res(7, &e)
#define RES_0_H() res(0, &h)
#define RES_1_H() res(1, &h)
#define RES_2_H() res(2, &h)
#define RES_3_H() res(3, &h)
#define RES_4_H() res(4, &h)
#define RES_5_H() res(5, &h)
#define RES_6_H() res(6, &h)
#define RES_7_H() res(7, &h)
#define RES_0_L() res(0, &l)
#define RES_1_L() res(1, &l)
#define RES_2_L() res(2, &l)
#define RES_3_L() res(3, &l)
#define RES_4_L() res(4, &l)
#define RES_5_L() res(5, &l)
#define RES_6_L() res(6, &l)
#define RES_7_L() res(7, &l)
#define RES_0_AHL() res_ahl(0)
#define RES_1_AHL() res_ahl(1)
#define RES_2_AHL() res_ahl(2)
#define RES_3_AHL() res_ahl(3)
#define RES_4_AHL() res_ahl(4)
#define RES_5_AHL() res_ahl(5)
#define RES_6_AHL() res_ahl(6)
#define RES_7_AHL() res_ahl(7)

// Jump

/**
 * Jump to address nn
 */
static void jump(uint16_t nn) {
    pc = nn;
    cpu_cycles += 12;
}

/**
 * Jump if z is reset
 */
static void jump_nz(uint16_t nn) {
    if (!FLAG_Z) {
        pc = nn;
    }

    cpu_cycles += 12;
}

/**
 * Jump if z is set
 */
static void jump_z(uint16_t nn) {
    if (FLAG_Z) {
        pc = nn;
    }

    cpu_cycles += 12;
}

/**
 * Jump if c is reset
 */
static void jump_nc(uint16_t nn) {
    if (!FLAG_C) {
        pc = nn;
    }

    cpu_cycles += 12;
}

/**
 * Jump if c is set
 */
static void jump_c(uint16_t nn) {
    if (FLAG_C) {
        pc = nn;
    }

    cpu_cycles += 12;
}

/**
 * Jump to pc + e
 */
static void jump_e(int8_t e) {
    pc += e;
    cpu_cycles += 8;
}

/**
 * Jump to pc + e if z is reset
 */
static void jump_e_nz(int8_t e) {
    if (!FLAG_Z) {
        pc += e;
    }

    cpu_cycles += 8;
}

/**
 * Jump to pc + e if z is set
 */
static void jump_e_z(int8_t e) {
    if (FLAG_Z) {
        pc += e;
    }

    cpu_cycles += 8;
}

/**
 * Jump to pc + e if c is reset
 */
static void jump_e_nc(int8_t e) {
    if (!FLAG_C) {
        pc += e;
    }

    cpu_cycles += 8;
}

/**
 * Jump to pc + e if c is set
 */
static void jump_e_c(int8_t e) {
    if (FLAG_C) {
        pc += e;
    }

    cpu_cycles += 8;
}

/**
 * Jump to address in hl
 */
static void jump_hl() {
    pc = HL_R();
    cpu_cycles += 4;
}

#define JP() jump(uarg16())

#define JP_NZ() jump_nz(uarg16())
#define JP_Z() jump_z(uarg16())
#define JP_NC() jump_nc(uarg16())
#define JP_C() jump_c(uarg16())

#define JP_HL() jump_hl()
#define JR() jump_e(arg8())

#define JR_NZ() jump_e_nz(arg8())
#define JR_Z() jump_e_z(arg8())
#define JR_NC() jump_e_nc(arg8())
#define JR_C() jump_e_c(arg8())

// Call and return

/**
 * Push address of next instruction to the stack and jump to nn
 */
static void call(uint16_t nn) {
    sp--;
    mem_write_byte(sp, pc >> 8);
    sp--;
    mem_write_byte(sp, pc & 0xFF);

    pc = nn;

    cpu_cycles += 12;
}

/**
 * Call address nn if z is reset
 */
static void call_nz(uint16_t nn) {
    if (!FLAG_Z) {
        call(nn);
    } else {
        cpu_cycles += 12;
    }
}

/**
 * Call address nn if z is set
 */
static void call_z(uint16_t nn) {
    if (FLAG_Z) {
        call(nn);
    } else {
        cpu_cycles += 12;
    }
}

/**
 * Call address nn if c is reset
 */
static void call_nc(uint16_t nn) {
    if (!FLAG_C) {
        call(nn);
    } else {
        cpu_cycles += 12;
    }
}

/**
 * Call address nn if c is set
 */
static void call_c(uint16_t nn) {
    if (FLAG_C) {
        call(nn);
    } else {
        cpu_cycles += 12;
    }
}

/**
 * Restart - push pc to stack and jump to 0 + n
 */
static void rst(uint8_t n) {
    call(n);
    cpu_cycles += 20;
}

/**
 * Return to address on stack
 */
static void ret() {
    pc = mem_read_byte(sp);
    sp++;
    pc |= mem_read_byte(sp) << 8;
    sp++;

    cpu_cycles += 8;
}

/**
 * Return if z is reset
 */
static void ret_nz() {
    if (!FLAG_Z) {
        ret();
    } else {
        cpu_cycles += 8;
    }
}

/**
 * Return if z is set
 */
static void ret_z() {
    if (FLAG_Z) {
        ret();
    } else {
        cpu_cycles += 8;
    }
}

/**
 * Return if c is reset
 */
static void ret_nc() {
    if (!FLAG_C) {
        ret();
    } else {
        cpu_cycles += 8;
    }
}

/**
 * Return if c is set
 */
static void ret_c() {
    if (FLAG_C) {
        ret();
    } else {
        cpu_cycles += 8;
    }
}

/**
 * Return & enable interrupts
 */
static void reti() {
    ret();
    get_gb_instance()->interrupts_enabled = 1;
}

#define CALL() call(uarg16())

#define CALL_NZ() call_nz(uarg16())
#define CALL_Z() call_z(uarg16())
#define CALL_NC() call_nc(uarg16())
#define CALL_C() call_c(uarg16())

#define RST_0() rst(0x00)
#define RST_1() rst(0x08)
#define RST_2() rst(0x10)
#define RST_3() rst(0x18)
#define RST_4() rst(0x20)
#define RST_5() rst(0x28)
#define RST_6() rst(0x30)
#define RST_7() rst(0x38)

#define RET() ret()
#define RET_NZ() ret_nz()
#define RET_Z() ret_z()
#define RET_NC() ret_nc()
#define RET_C() ret_c()

#define RETI() reti()

#define DI() get_gb_instance()->interrupts_enabled = 0; cpu_cycles += 4
#define EI() get_gb_instance()->interrupts_enabled = 1; cpu_cycles += 4

// General-purpose Arithmetic Operations and CPU Control

/**
 * Decimal adjust acc
 */
static void daa() {

}

/**
 * One's compliment of A
 */
static void cpl() {
    a = ~a;
    cpu_cycles += 4;
}

/**
 * Compliment carry flag
 */
static void ccf() {
    if (FLAG_C) {
        C_RESET();
    } else {
        C_SET();
    }
}

/**
 * No operation
 */
static void nop() {
    cpu_cycles += 4;
}

/**
 * Halt. Wait until interrupt occurs
 */
static void halt() {
    // TODO: what to do here
}

/**
 * Stop. halt cpu & lcd until button press
 */
static void stop() {
    // TODO
}

#define DAA() daa()
#define CPL() cpl()
#define CCF() ccf()
#define SCF() C_SET()

#define NOP() nop()
#define HALT() halt()
#define STOP() stop()

/* END INSTRUCTIONS */

void cpu_init() {
    a = 0;
    b = 0;
    c = 0;
    d = 0;
    e = 0;
    f = 0;
    l = 0;
    h = 0;

    sp = 0xFFFE;
    pc = 0x0;

    printf("CPU init\r\n");
}

int cpu_tick() {
    // Opcode
    uint8_t opcode = mem_read_byte(pc++);

    #if OPCODE_DEBUG
        if (!get_gb_instance()->in_bios) printf("OP: %X pc: %X\r\n", opcode, pc - 1);
    #endif

    // Instruction decoding
    switch (opcode) {
        /* --- 0x0+ --- */
        case 0x00:
            NOP();
            break;

        case 0x01:
            LD_BC_NN();
            break;

        case 0x02:
            LD_ABC_A();
            break;

        case 0x03:
            INC_BC();
            break;
        
        case 0x04:
            INC_B();
            break;

        case 0x05:
            DEC_B();
            break;

        case 0x06:
            LD_B_N();
            break;

        case 0x07:
            RLCA();
            break;

        case 0x08:
            LD_ANN_SP();
            break;

        case 0x09:
            ADD_HL_BC();
            break;

        case 0x0A:
            LD_A_ABC();
            break;
        
        case 0x0B:
            DEC_BC();
            break;

        case 0x0C:
            INC_C();
            break;

        case 0x0D:
            DEC_C();
            break;

        case 0x0E:
            LD_C_N();
            break;

        case 0x0F:
            RRCA();
            break;

        /* --- 0x1+ --- */

        case 0x10:
            STOP();
            break;

        case 0x11:
            LD_DE_NN();
            break;

        case 0x12:
            LD_ADE_A();
            break;

        case 0x13:
            INC_DE();
            break;

        case 0x14:
            INC_D();
            break;

        case 0x15:
            DEC_D();
            break;

        case 0x16:
            LD_D_N();
            break;

        case 0x17:
            RLA();
            break;

        case 0x18:
            JR();
            break;

        case 0x19:
            ADD_HL_DE();
            break;

        case 0x1A:
            LD_A_ADE();
            break;

        case 0x1B:
            DEC_DE();
            break;

        case 0x1C:
            INC_E();
            break;

        case 0x1D:
            DEC_E();
            break;

        case 0x1E:
            LD_E_N();
            break;

        case 0x1F:
            RRA();
            break;

        /* --- 0x2+ --- */
        case 0x20:
            JR_NZ();
            break;

        case 0x21:
            LD_HL_NN();
            break;

        case 0x22:
            LD_AHLI_A();
            break;

        case 0x23:
            INC_HL();
            break;

        case 0x24:
            INC_H();
            break;

        case 0x25:
            DEC_H();
            break;

        case 0x26:
            LD_H_N();
            break;

        case 0x27:
            DAA();
            break;

        case 0x28:
            JR_Z();
            break;

        case 0x29:
            ADD_HL_HL();
            break;

        case 0x2A:
            LD_A_AHLI();
            break;

        case 0x2B:
            DEC_HL();
            break;

        case 0x2C:
            INC_L();
            break;

        case 0x2D:
            DEC_L();
            break;

        case 0x2E:
            LD_L_N();
            break;

        case 0x2F:
            CPL();
            break;

        /* --- 0x3+ --- */
        case 0x30:
            JR_NC();
            break;

        case 0x31:
            LD_SP_NN();
            break;

        case 0x32:
            LD_AHLD_A();
            break;

        case 0x33:
            INC_SP();
            break;

        case 0x34:
            INC_AHL();
            break;

        case 0x35:
            DEC_AHL();
            break;

        case 0x36:
            LD_AHL_N();
            break;

        case 0x37:
            SCF();
            break;

        case 0x38:
            JR_C();
            break;

        case 0x39:
            ADD_HL_SP();
            break;

        case 0x3A:
            LD_A_AHLD();
            break;

        case 0x3B:
            DEC_SP();
            break;

        case 0x3C:
            INC_A();
            break;

        case 0x3D:
            DEC_A();
            break;

        case 0x3E:
            LD_A_N();
            break;

        case 0x3F:
            CCF();
            break;

        /* --- 0x4+ --- */
        case 0x40:
            LD_B_B();
            break;

        case 0x41:
            LD_B_C();
            break;

        case 0x42:
            LD_B_D();
            break;

        case 0x43:
            LD_B_E();
            break;

        case 0x44:
            LD_B_H();
            break;

        case 0x45:
            LD_B_L();
            break;

        case 0x46:
            LD_B_AHL();
            break;

        case 0x47:
            LD_B_A();
            break;

        case 0x48:
            LD_C_B();
            break;

        case 0x49:
            LD_C_C();
            break;

        case 0x4A:
            LD_C_D();
            break;

        case 0x4B:
            LD_C_E();
            break;

        case 0x4C:
            LD_C_H();
            break;

        case 0x4D:
            LD_C_L();
            break;

        case 0x4E:
            LD_C_AHL();
            break;

        case 0x4F:
            LD_C_A();
            break;

        /* --- 0x5+ --- */
        case 0x50:
            LD_D_B();
            break;

        case 0x51:
            LD_D_C();
            break;

        case 0x52:
            LD_D_D();
            break;

        case 0x53:
            LD_D_E();
            break;

        case 0x54:
            LD_D_H();
            break;

        case 0x55:
            LD_D_L();
            break;

        case 0x56:
            LD_D_AHL();
            break;

        case 0x57:
            LD_D_A();
            break;

        case 0x58:
            LD_E_B();
            break;

        case 0x59:
            LD_E_C();
            break;

        case 0x5A:
            LD_E_D();
            break;

        case 0x5B:
            LD_E_E();
            break;

        case 0x5C:
            LD_E_H();
            break;

        case 0x5D:
            LD_E_L();
            break;

        case 0x5E:
            LD_E_AHL();
            break;

        case 0x5F:
            LD_E_A();
            break;

        /* --- 0x6+ --- */
        case 0x60:
            LD_H_B();
            break;

        case 0x61:
            LD_H_C();
            break;

        case 0x62:
            LD_H_D();
            break;

        case 0x63:
            LD_H_E();
            break;

        case 0x64:
            LD_H_H();
            break;

        case 0x65:
            LD_H_L();
            break;

        case 0x66:
            LD_H_AHL();
            break;

        case 0x67:
            LD_H_A();
            break;

        case 0x68:
            LD_L_B();
            break;

        case 0x69:
            LD_L_C();
            break;

        case 0x6A:
            LD_L_D();
            break;

        case 0x6B:
            LD_L_E();
            break;

        case 0x6C:
            LD_L_H();
            break;

        case 0x6D:
            LD_L_L();
            break;

        case 0x6E:
            LD_L_AHL();
            break;

        case 0x6F:
            LD_L_A();
            break;

        /* --- 0x7+ --- */
        case 0x70:
            LD_AHL_B();
            break;

        case 0x71:
            LD_AHL_C();
            break;

        case 0x72:
            LD_AHL_D();
            break;

        case 0x73:
            LD_AHL_E();
            break;

        case 0x74:
            LD_AHL_H();
            break;

        case 0x75:
            LD_AHL_L();
            break;

        case 0x76:
            HALT();
            break;

        case 0x77:
            LD_AHL_A();
            break;

        case 0x78:
            LD_A_B();
            break;

        case 0x79:
            LD_A_C();
            break;

        case 0x7A:
            LD_A_D();
            break;

        case 0x7B:
            LD_A_E();
            break;

        case 0x7C:
            LD_A_H();
            break;

        case 0x7D:
            LD_A_L();
            break;

        case 0x7E:
            LD_A_AHL();
            break;

        case 0x7F:
            LD_A_A();
            break;

        /* --- 0x8+ --- */
        case 0x80:
            ADD_A_B();
            break;

        case 0x81:
            ADD_A_C();
            break;

        case 0x82:
            ADD_A_D();
            break;

        case 0x83:
            ADD_A_E();
            break;

        case 0x84:
            ADD_A_H();
            break;

        case 0x85:
            ADD_A_L();
            break;

        case 0x86:
            ADD_A_AHL();
            break;
        
        case 0x87:
            ADD_A_A();
            break;

        case 0x88:
            ADC_A_B();
            break;

        case 0x89:
            ADC_A_C();
            break;

        case 0x8A:
            ADC_A_D();
            break;

        case 0x8B:
            ADC_A_E();
            break;

        case 0x8C:
            ADC_A_H();
            break;

        case 0x8D:
            ADC_A_L();
            break;

        case 0x8E:
            ADC_A_AHL();
            break;

        case 0x8F:
            ADC_A_A();
            break;

        /* --- 0x9+ --- */
        case 0x90:
            SUB_A_B();
            break;

        case 0x91:
            SUB_A_C();
            break;

        case 0x92:
            SUB_A_D();
            break;

        case 0x93:
            SUB_A_E();
            break;

        case 0x94:
            SUB_A_H();
            break;

        case 0x95:
            SUB_A_L();
            break;

        case 0x96:
            SUB_A_AHL();
            break;
        
        case 0x97:
            SUB_A_A();
            break;

        case 0x98:
            SBC_A_B();
            break;

        case 0x99:
            SBC_A_C();
            break;

        case 0x9A:
            SBC_A_D();
            break;

        case 0x9B:
            SBC_A_E();
            break;

        case 0x9C:
            SBC_A_H();
            break;

        case 0x9D:
            SBC_A_L();
            break;

        case 0x9E:
            SBC_A_AHL();
            break;

        case 0x9F:
            SBC_A_A();
            break;

        /* --- 0xA+ --- */
        case 0xA0:
            AND_A_B();
            break;

        case 0xA1:
            AND_A_C();
            break;

        case 0xA2:
            AND_A_D();
            break;

        case 0xA3:
            AND_A_E();
            break;

        case 0xA4:
            AND_A_H();
            break;

        case 0xA5:
            AND_A_L();
            break;

        case 0xA6:
            AND_A_AHL();
            break;
        
        case 0xA7:
            AND_A_A();
            break;

        case 0xA8:
            XOR_A_B();
            break;

        case 0xA9:
            XOR_A_C();
            break;

        case 0xAA:
            XOR_A_D();
            break;

        case 0xAB:
            XOR_A_E();
            break;

        case 0xAC:
            XOR_A_H();
            break;

        case 0xAD:
            XOR_A_L();
            break;

        case 0xAE:
            XOR_A_AHL();
            break;

        case 0xAF:
            XOR_A_A();
            break;

        /* --- 0xB+ --- */
        case 0xB0:
            OR_A_B();
            break;

        case 0xB1:
            OR_A_C();
            break;

        case 0xB2:
            OR_A_D();
            break;

        case 0xB3:
            OR_A_E();
            break;

        case 0xB4:
            OR_A_H();
            break;

        case 0xB5:
            OR_A_L();
            break;

        case 0xB6:
            OR_A_AHL();
            break;

        case 0xB7:
            OR_A_A();
            break;

        case 0xB8:
            CP_A_B();
            break;

        case 0xB9:
            CP_A_C();
            break;

        case 0xBA:
            CP_A_D();
            break;

        case 0xBB:
            CP_A_E();
            break;

        case 0xBC:
            CP_A_H();
            break;

        case 0xBD:
            CP_A_L();
            break;

        case 0xBE:
            CP_A_AHL();
            break;

        case 0xBF:
            CP_A_A();
            break;

        /* --- 0xC+ --- */
        case 0xC0:
            RET_NZ();
            break;

        case 0xC1:
            POP_BC();
            break;

        case 0xC2:
            JP_NZ();
            break;

        case 0xC3:
            JP();
            break;

        case 0xC4:
            CALL_NZ();
            break;

        case 0xC5:
            PUSH_BC();
            break;

        case 0xC6:
            ADD_A_N();
            break;

        case 0xC7:
            RST_0();
            break;

        case 0xC8:
            RET_Z();
            break;

        case 0xC9:
            RET();
            break;

        case 0xCA:
            JP_Z();
            break;

        case 0xCB:
            // Go into CB prefixed table
            ;
            uint8_t opcode_second = mem_read_byte(pc++);

            #if OPCODE_DEBUG
                printf("OP: CB%X\r\n", opcode_second);
            #endif

            switch (opcode_second) {
                /* --- 0x0+ --- */

                case 0x00:
                    RLC_B();
                    break;

                /* --- 0x1+ --- */

                case 0x11:
                    RL_C();
                    break;

                /* --- 0x2+ --- */

                /* --- 0x3+ --- */

                case 0x37:
                    SWAP_A();
                    break;

                /* --- 0x4+ --- */

                /* --- 0x5+ --- */

                /* --- 0x6+ --- */

                /* --- 0x7+ --- */

                case 0x7C:
                    BIT_7_H();
                    break;

                /* --- 0x8+ --- */

                case 0x87:
                    RES_0_A();
                    break;

                /* --- 0x9+ --- */

                /* --- 0xA+ --- */

                /* --- 0xB+ --- */

                /* --- 0xC+ --- */
                case 0xCF:
                    SET_1_A();
                    break;
                
                /* --- 0xD+ --- */

                /* --- 0xE+ --- */

                /* --- 0xF+ --- */

                default:
                    // Instruction not implemeneted yet
                    printf("Instruction with opcode CB%X undefined\r\n", opcode_second);
                    return TICK_FAIL;
                    break;
            }

            break;

        case 0xCC:
            CALL_Z();
            break;

        case 0xCD:
            CALL();
            break;

        case 0xCE:
            ADC_A_N();
            break;

        case 0xCF:
            RST_1();
            break;

        /* --- 0xD+ --- */

        case 0xD0:
            RET_NC();
            break;

        case 0xD1:
            POP_DE();
            break;

        case 0xD2:
            JP_NC();
            break;

        case 0xD3:
            break;

        case 0xD4:
            CALL_NC();
            break;

        case 0xD5:
            PUSH_DE();
            break;

        case 0xD6:
            SUB_A_N();
            break;

        case 0xD7:
            RST_2();
            break;

        case 0xD8:
            RET_C();
            break;

        case 0xD9:
            RETI();
            break;

        case 0xDA:
            JP_C();
            break;

        case 0xDB:
            break;

        case 0xDC:
            CALL_C();
            break;

        case 0xDD:
            break;

        case 0xDE:
            SBC_A_N();
            break;

        case 0xDF:
            RST_3();
            break;

        /* --- 0xE+ --- */
        case 0xE0:
            LD_AN_A();
            break;

        case 0xE1:
            POP_HL();
            break;

        case 0xE2:
            LD_AC_A();
            break;

        case 0xE3:
        case 0xE4:
            break;

        case 0xE5:
            PUSH_HL();
            break;

        case 0xE6:
            AND_A_N();
            break;

        case 0xE7:
            RST_4();
            break;

        case 0xE8:
            ADD_SP_E();
            break;

        case 0xE9:
            JP_HL();
            break;

        case 0xEA:
            LD_ANN_A();
            break;

        case 0xEB:
        case 0xEC:
        case 0xED:
            break;

        case 0xEE:
            XOR_A_N();
            break;

        case 0xEF:
            RST_5();
            break;

        /* --- 0xF+ --- */
        case 0xF0:
            LD_A_AN();
            break;

        case 0xF1:
            POP_AF();
            break;

        case 0xF2:
            LD_A_AC();
            break;

        case 0xF3:
            DI();

        case 0xF4:
            break;

        case 0xF5:
            PUSH_AF();
            break;

        case 0xF6:
            OR_A_N();
            break;

        case 0xF7:
            RST_6();
            break;
        
        case 0xF8:
            LDHL_SP_E();
            break;

        case 0xF9:
            LD_SP_HL();
            break;

        case 0xFA:
            LD_A_ANN();
            break;

        case 0xFB:
            EI();
            break;

        case 0xFC:
        case 0xFD:
            break;

        case 0xFE:
            CP_A_N();
            break;

        case 0xFF:
            RST_7();
            break;
        
        default:
            // Instruction not implemeneted yet
            printf("Instruction with opcode %X undefined\r\n", opcode);
            return TICK_FAIL;
            break;
    }

    // OK
    return TICK_PASS;
}

uint16_t get_program_counter() {
    return pc;
}

uint32_t get_ticks() {
    return cpu_cycles;
}