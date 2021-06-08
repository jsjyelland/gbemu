#include <cpu.h>

/* HELPER FUNCTIONS */

#define REG_A_PARAM 0b111
#define REG_B_PARAM 0b000
#define REG_C_PARAM 0b001
#define REG_D_PARAM 0b010
#define REG_E_PARAM 0b011
#define REG_H_PARAM 0b100
#define REG_L_PARAM 0b101

#define OPCODE_PARAM_HIGH(opcode) ((opcode >> 3) & 0b111)
#define OPCODE_PARAM_LOW(opcode) (opcode & 0b111)

/**
 * Return a pointer to the cpu register given the opcode param
 */
static uint8_t* cpu_register_for_param(gb_t *gb, uint8_t param) {
    switch (param) {
        case REG_A_PARAM:
            return &(gb->cpu.a);

        case REG_B_PARAM:
            return &(gb->cpu.b);

        case REG_C_PARAM:
            return &(gb->cpu.c);

        case REG_D_PARAM:
            return &(gb->cpu.d);

        case REG_E_PARAM:
            return &(gb->cpu.e);

        case REG_H_PARAM:
            return &(gb->cpu.h);

        case REG_L_PARAM:
            return &(gb->cpu.l);

        default:
            return NULL;
    }
}

#define CC_NZ 0b00
#define CC_Z 0b01
#define CC_NC 0b10
#define CC_C 0b11

/**
 * Return the flag boolean expression for a cc value
 */
static uint8_t cpu_cc_for_param(gb_t *gb, uint8_t param) {
    switch (param) {
        case CC_NZ:
            return !gb->cpu.flag_z;

        case CC_Z:
            return gb->cpu.flag_z;

        case CC_NC:
            return !gb->cpu.flag_c;

        case CC_C:
            return gb->cpu.flag_c;

        default:
            return 0;
    }
}

/**
 * Read a byte from the memory at the program counter,
 * incrementing the counter
 */
static uint8_t cpu_read_program(gb_t *gb) {
    uint8_t val = mem_read_byte(gb, (gb->cpu.pc)++);
    return val;
}


/**
 * Retrieve unsigned 8-bit immediate argument
 */
uint8_t cpu_read_n(gb_t *gb) {
    #if OPCODE_DEBUG
        #if !OPCODE_BIOS_DEBUG
            if (!gb->in_bios)
        #endif
        if (gb->cpu.pc < 0x02ED || gb->cpu.pc > 0x02F1) printf("\t%X", mem_read_byte(gb, gb->cpu.pc));
    #endif

    return cpu_read_program(gb);
}

/**
 * Retrieve signed 8-bit immediate argument
 */
int8_t cpu_read_e(gb_t *gb) {
    #if OPCODE_DEBUG
        #if !OPCODE_BIOS_DEBUG
            if (!gb->in_bios)
        #endif
        if (gb->cpu.pc < 0x02ED || gb->cpu.pc > 0x02F1) printf("\t%X", mem_read_byte(gb, gb->cpu.pc));
    #endif

    return (int8_t)cpu_read_program(gb);
}

/**
 * Retrieve unsigned 16-bit immediate argument
 */
uint16_t cpu_read_nn(gb_t *gb) {
    #if OPCODE_DEBUG
        #if !OPCODE_BIOS_DEBUG
            if (!gb->in_bios)
        #endif
        if (gb->cpu.pc < 0x02ED || gb->cpu.pc > 0x02F1) printf("\t%X", mem_read_word(gb, gb->cpu.pc));
    #endif
    
    uint16_t arg = mem_read_word(gb, gb->cpu.pc);
    gb->cpu.pc += 2;

    return arg;
}

/**
 * Check for carry in add (a + b)
 */
static void carry_check_add(gb_t *gb, uint8_t a, uint8_t b) {
    gb->cpu.flag_c = (((uint16_t)a + (uint16_t)b) > 0xFF);
}

/**
 * Check for carry in 16 bit add (a + b)
 */
static void carry_check_add_word(gb_t *gb, uint16_t a, uint16_t b) {
    gb->cpu.flag_c = (((uint32_t)a + (uint32_t)b > 0xFFFF));
}

/**
 * Check for carry in subtract (a - b)
 */
static void carry_check_sub(gb_t *gb, uint8_t a, uint8_t b) {
    gb->cpu.flag_c = (a < b);
}

/**
 * Check for carry in 16-bit subtract (a - b)
 */
static void carry_check_sub_word(gb_t *gb, uint16_t a, uint16_t b) {
    gb->cpu.flag_c = (a < b);
}

/**
 * Check for zero result and set flag
 */
static void zero_check(gb_t *gb, uint16_t result) {
    gb->cpu.flag_z = result == 0;
}

/**
 * Check for half carry in add (a + b)
 */
static void half_carry_check_add(gb_t *gb, uint8_t a, uint8_t b) {
    gb->cpu.flag_h = ((a & 0xF) + (b & 0xF)) > 0xF;
}

/**
 * Check for half carry in 16-bit add (a + b)
 */
static void half_carry_check_add_word(gb_t *gb, uint16_t a, uint16_t b) {
    gb->cpu.flag_h = ((a & 0xFFF) + (b & 0xFFF)) > 0xFFF;
}


/**
 * Check for half carry in subtract (a - b)
 */
static void half_carry_check_sub(gb_t *gb, uint8_t a, uint8_t b) {
    gb->cpu.flag_h = ((a & 0xF) < (b & 0xF));
}

/* BEGIN INSTRUCTIONS REFACTOR */

typedef uint8_t cpu_instruction_t(gb_t *gb, uint8_t opcode);

// Instruction naming convention
// 
// r -> 8-bit register
// rr -> 16-bit register
// n -> immediate 8-bit unsigned
// nn -> immediate 16-bit unsigned
// e -> immediate 8-bit signed
// m<x> -> value at memory address of <x>
// 

// 8-bit loads

/**
 * Load register r2 into r1.
 * 
 * 1 machine cycle
 */
static uint8_t ld_r_r(gb_t *gb, uint8_t opcode) {
    uint8_t *r1 = cpu_register_for_param(gb, OPCODE_PARAM_HIGH(opcode));
    uint8_t *r2 = cpu_register_for_param(gb, OPCODE_PARAM_LOW(opcode));

    *r1 = *r2;

    return 1;
}

/**
 * Load immediate value into r
 * 
 * 2 machine cycles
 */
static uint8_t ld_r_n(gb_t *gb, uint8_t opcode) {
    uint8_t *r = cpu_register_for_param(gb, OPCODE_PARAM_HIGH(opcode));

    *r = cpu_read_n(gb);

    return 2;
}

/**
 * Load value at (hl) into r
 * 
 * 2 machine cycles
 */
static uint8_t ld_r_mhl(gb_t *gb, uint8_t opcode) {
    uint8_t *r = cpu_register_for_param(gb, OPCODE_PARAM_HIGH(opcode));

    *r = mem_read_byte(gb, gb->cpu.hl);

    return 2;
}

/**
 * Load value in r into (hl)
 *
 * 2 machine cycles
 */
static uint8_t ld_mhl_r(gb_t *gb, uint8_t opcode) {
    uint8_t *r = cpu_register_for_param(gb, OPCODE_PARAM_LOW(opcode));

    mem_write_byte(gb, gb->cpu.hl, *r);

    return 2;
}

/**
 * Load immediate value into (hl)
 * 
 * 3 machine cycles
 */
static uint8_t ld_mhl_n(gb_t *gb, uint8_t opcode) {
    mem_write_byte(gb, gb->cpu.hl, cpu_read_n(gb));

    return 3;
}

/**
 * Load (bc) into a
 * 
 * 2 machine cycles
 */
static uint8_t ld_a_mbc(gb_t *gb, uint8_t opcode) {
    gb->cpu.a = mem_read_byte(gb, gb->cpu.bc);

    return 2;
}

/**
 * Load (de) into a
 *
 * 2 machine cycles
 */
static uint8_t ld_a_mde(gb_t *gb, uint8_t opcode) {
    gb->cpu.a = mem_read_byte(gb, gb->cpu.de);

    return 2;
}

/**
 * Load (nn) into a
 * 
 * 4 machine cycles
 */
static uint8_t ld_a_mnn(gb_t *gb, uint8_t opcode) {
    gb->cpu.a = cpu_read_nn(gb);

    return 4;
}

/**
 * Load a into (bc)
 * 
 * 2 machine cycles
 */
static uint8_t ld_mbc_a(gb_t *gb, uint8_t opcode) {
    mem_write_byte(gb, gb->cpu.bc, gb->cpu.a);

    return 2;
}

/**
 * Load a into (de)
 * 
 * 2 machine cycles
 */
static uint8_t ld_mde_a(gb_t *gb, uint8_t opcode) {
    mem_write_byte(gb, gb->cpu.de, gb->cpu.a);

    return 2;
}

/**
 * Load a into (nn)
 * 
 * 4 machine cycles
 */
static uint8_t ld_mnn_a(gb_t *gb, uint8_t opcode) {
    mem_write_byte(gb, cpu_read_nn(gb), gb->cpu.a);

    return 4;
}

/**
 * Load high (0xFF00 + n) into a
 * 
 * 3 machine cycles
 */
static uint8_t ldh_a_mn(gb_t *gb, uint8_t opcode) {
    gb->cpu.a = mem_read_byte(gb, 0xFF00 + cpu_read_n(gb));

    return 3;
}

/**
 * Load high a into (0xFF00 + n)
 * 
 * 3 machine cycles
 */
static uint8_t ldh_mn_a(gb_t *gb, uint8_t opcode) {
    mem_write_byte(gb, 0xFF00 + cpu_read_n(gb), gb->cpu.a);

    return 3;
}

/**
 * Load high (0xFF00 + c) into a
 * 
 * 2 machine cycles
 */
static uint8_t ldh_a_mc(gb_t *gb, uint8_t opcode) {
    gb->cpu.a = mem_read_byte(gb, 0xFF00 + gb->cpu.c);

    return 2;
}

/**
 * Load high a into (0xFF00 + c)
 * 
 * 2 machine cycles
 */
static uint8_t ldh_mc_a(gb_t *gb, uint8_t opcode) {
    mem_write_byte(gb, 0xFF00 + gb->cpu.c, gb->cpu.a);

    return 3;
}

/**
 * Load a into (hl) and increment hl
 * 
 * 2 machine cycles
 */
static uint8_t ldi_mhl_a(gb_t *gb, uint8_t opcode) {
    mem_write_byte(gb, (gb->cpu.hl)++, gb->cpu.a);

    return 2;
}

/**
 * Load (hl) into a and increment hl
 * 
 * 2 machine cycles
 */
static uint8_t ldi_a_mhl(gb_t *gb, uint8_t opcode) {
    gb->cpu.a = mem_read_byte(gb, (gb->cpu.hl)++);

    return 2;
}

/**
 * Load a into (hl) and decrement hl
 * 
 * 2 machine cycles
 */
static uint8_t ldd_mhl_a(gb_t *gb, uint8_t opcode) {
    mem_write_byte(gb, (gb->cpu.hl)--, gb->cpu.a);

    return 2;
}

/**
 * Load (hl) into a and decrement hl
 * 
 * 2 machine cycles
 */
static uint8_t ldd_a_mhl(gb_t *gb, uint8_t opcode) {
    gb->cpu.a = mem_read_byte(gb, (gb->cpu.hl)--);

    return 2;
}

// 16-bit loads

/**
 * Load 16 bit immediate into rr
 * 
 * 3 machine cycles
 */
static uint8_t ld_rr_nn(gb_t *gb, uint8_t opcode) {
    uint16_t *r;

    switch (OPCODE_PARAM_HIGH(opcode) >> 1) {
        case 0b00:
            r = &gb->cpu.bc;
            break;

        case 0b01:
            r = &gb->cpu.de;
            break;

        case 0b10:
            r = &gb->cpu.hl;
            break;

        case 0b11:
            r = &gb->cpu.sp;
            break;
    }

    *r = cpu_read_nn(gb);

    return 3;
}

/**
 * Load sp into (nn)
 * 
 * 5 machine cycles
 */
static uint8_t ld_mnn_sp(gb_t *gb, uint8_t opcode) {
    mem_write_word(gb, cpu_read_nn(gb), gb->cpu.sp);

    return 5;
}

/**
 * Load hl into sp
 * 
 * 2 machine cycles
 */
static uint8_t ld_sp_hl(gb_t *gb, uint8_t opcode) {
    gb->cpu.sp = gb->cpu.hl;

    return 2;
}

/**
 * Push rr onto the stack
 * 
 * 4 machine cycles
 */
static uint8_t push_rr(gb_t *gb, uint8_t opcode) {
    uint16_t *r;

    switch (OPCODE_PARAM_HIGH(opcode) >> 1) {
        case 0b00:
            r = &gb->cpu.bc;
            break;

        case 0b01:
            r = &gb->cpu.de;
            break;

        case 0b10:
            r = &gb->cpu.hl;
            break;

        case 0b11:
            r = &gb->cpu.af;
            break;
    }

    gb->cpu.sp -= 2;
    mem_write_word(gb, gb->cpu.sp, *r);

    return 4;
}

/**
 * Pop rr off the stack
 * 
 * 3 machine cycles
 */
static uint8_t pop_rr(gb_t *gb, uint8_t opcode) {
    uint16_t *r;

    switch (OPCODE_PARAM_HIGH(opcode) >> 1) {
        case 0b00:
            r = &gb->cpu.bc;
            break;

        case 0b01:
            r = &gb->cpu.de;
            break;

        case 0b10:
            r = &gb->cpu.hl;
            break;

        case 0b11:
            r = &gb->cpu.af;
            break;
    }

    *r = mem_read_word(gb, gb->cpu.sp);
    gb->cpu.sp += 2;

    return 3;
}

/**
 * Load sp + immediate signed into hl
 *
 * 3 machine cycles
 */
static uint8_t ld_hl_spe(gb_t *gb, uint8_t opcode) {
    gb->cpu.f = 0;

    int8_t e = cpu_read_e(gb);

    if (e >= 0) {
        carry_check_add_word(gb, gb->cpu.sp, e);
    } else {
        carry_check_sub_word(gb, gb->cpu.sp, -e);
    }

    // Half carry behaviour unknown, won't bother implementing for now

    gb->cpu.hl = gb->cpu.sp + e;

    return 3;
}

// 8-bit ALU

/**
 * Helper function for adding n to a
 */
static void add_helper(gb_t *gb, uint8_t n) {
    gb->cpu.f = 0;

    carry_check_add(gb, gb->cpu.a, n);
    half_carry_check_add(gb, gb->cpu.a, n);

    gb->cpu.a = gb->cpu.a + n;

    zero_check(gb, gb->cpu.a);
}

/**
 * Add r to a and store in a
 * 
 * 1 machine cycle
 */
static uint8_t add_a_r(gb_t *gb, uint8_t opcode) {
    add_helper(gb, *cpu_register_for_param(gb, OPCODE_PARAM_LOW(opcode)));

    return 1;
}

/**
 * Add immediate to a and store in a
 * 
 * 2 machine cycles
 */
static uint8_t add_a_n(gb_t *gb, uint8_t opcode) {
    add_helper(gb, cpu_read_n(gb));

    return 2;
}

/**
 * Add (hl) to a and store in a
 * 
 * 2 machine cycles
 */
static uint8_t add_a_mhl(gb_t *gb, uint8_t opcode) {
    add_helper(gb, mem_read_byte(gb, gb->cpu.hl));

    return 2;
}

/**
 * Add with carry r to a and store in a
 * 
 * 1 machine cycle
 */
static uint8_t adc_a_r(gb_t *gb, uint8_t opcode) {
    add_helper(gb, gb->cpu.flag_c + *cpu_register_for_param(gb, OPCODE_PARAM_LOW(opcode)));

    return 1;
}

/**
 * Add with carry immediate to a and store in a
 * 
 * 2 machine cycles
 */
static uint8_t adc_a_n(gb_t *gb, uint8_t opcode) {
    add_helper(gb, gb->cpu.flag_c + cpu_read_n(gb));

    return 2;
}

/**
 * Add with carry (hl) to a and store in a
 * 
 * 2 machine cycles
 */
static uint8_t adc_a_mhl(gb_t *gb, uint8_t opcode) {
    add_helper(gb, gb->cpu.flag_c + mem_read_byte(gb, gb->cpu.hl));

    return 2;
}

/**
 * Helper function for subtracting n from a
 */
static void sub_helper(gb_t *gb, uint8_t n) {
    gb->cpu.f = 0;
    gb->cpu.flag_n = 1;

    carry_check_sub(gb, gb->cpu.a, n);
    half_carry_check_sub(gb, gb->cpu.a, n);

    gb->cpu.a = gb->cpu.a - n;

    zero_check(gb, gb->cpu.a);
}

/**
 * Subtract r from a and store in a
 * 
 * 1 machine cycle
 */
static uint8_t sub_a_r(gb_t *gb, uint8_t opcode) {
    sub_helper(gb, *cpu_register_for_param(gb, OPCODE_PARAM_LOW(opcode)));

    return 1;
}

/**
 * Subtract immediate from a and store in a
 * 
 * 2 machine cycles
 */
static uint8_t sub_a_n(gb_t *gb, uint8_t opcode) {
    sub_helper(gb, cpu_read_n(gb));

    return 2;
}

/**
 * Subtract (hl) from a and store in a
 * 
 * 2 machine cycles
 */
static uint8_t sub_a_mhl(gb_t *gb, uint8_t opcode) {
    sub_helper(gb, mem_read_byte(gb, gb->cpu.hl));

    return 2;
}

/**
 * Subtract with carry r from a and store in a
 * 
 * 1 machine cycle
 */
static uint8_t sbc_a_r(gb_t *gb, uint8_t opcode) {
    sub_helper(gb, *cpu_register_for_param(gb, OPCODE_PARAM_LOW(opcode)) - gb->cpu.flag_c);

    return 1;
}

/**
 * Subtract with carry immediate from a and store in a
 * 
 * 2 machine cycles
 */
static uint8_t sbc_a_n(gb_t *gb, uint8_t opcode) {
    sub_helper(gb, cpu_read_n(gb) - gb->cpu.flag_c);

    return 2;
}

/**
 * Subtract with carry (hl) from a and store in a
 * 
 * 2 machine cycles
 */
static uint8_t sbc_a_mhl(gb_t *gb, uint8_t opcode) {
    sub_helper(gb, mem_read_byte(gb, gb->cpu.hl) - gb->cpu.flag_c);

    return 2;
}

/**
 * And helper for a and n
 */
static void and_helper(gb_t *gb, uint8_t n) {
    gb->cpu.f = 0;
    gb->cpu.flag_h = 1;
    gb->cpu.a &= n;
    zero_check(gb, gb->cpu.a);
}

/**
 * And a with r
 * 
 * 1 machine cycle
 */
static uint8_t and_a_r(gb_t *gb, uint8_t opcode) {
    and_helper(gb, *cpu_register_for_param(gb, OPCODE_PARAM_LOW(opcode)));

    return 1;
}

/**
 * And a with immediate
 * 
 * 2 machine cycles
 */
static uint8_t and_a_n(gb_t *gb, uint8_t opcode) {
    and_helper(gb, cpu_read_n(gb));

    return 2;
}

/**
 * And a with (hl)
 * 
 * 2 machine cycles
 */
static uint8_t and_a_mhl(gb_t *gb, uint8_t opcode) {
    and_helper(gb, mem_read_byte(gb, gb->cpu.hl));

    return 2;
}

/**
 * Or helper for a and n
 */
static void or_helper(gb_t *gb, uint8_t n) {
    gb->cpu.f = 0;
    gb->cpu.a |= n;
    zero_check(gb, gb->cpu.a);
}

/**
 * Or a with r
 * 
 * 1 machine cycle
 */
static uint8_t or_a_r(gb_t *gb, uint8_t opcode) {
    or_helper(gb, *cpu_register_for_param(gb, OPCODE_PARAM_LOW(opcode)));

    return 1;
}

/**
 * Or a with immediate
 * 
 * 2 machine cycles
 */
static uint8_t or_a_n(gb_t *gb, uint8_t opcode) {
    or_helper(gb, cpu_read_n(gb));

    return 2;
}

/**
 * Or a with (hl)
 * 
 * 2 machine cycles
 */
static uint8_t or_a_mhl(gb_t *gb, uint8_t opcode) {
    or_helper(gb, mem_read_byte(gb, gb->cpu.hl));

    return 2;
}

/**
 * Xor helper for a and n
 */
static void xor_helper(gb_t *gb, uint8_t n) {
    gb->cpu.f = 0;
    gb->cpu.a ^= n;
    zero_check(gb, gb->cpu.a);
}

/**
 * Xor a with r
 * 
 * 1 machine cycle
 */
static uint8_t xor_a_r(gb_t *gb, uint8_t opcode) {
    xor_helper(gb, *cpu_register_for_param(gb, OPCODE_PARAM_LOW(opcode)));

    return 1;
}

/**
 * Xor a with immediate
 * 
 * 2 machine cycles
 */
static uint8_t xor_a_n(gb_t *gb, uint8_t opcode) {
    xor_helper(gb, cpu_read_n(gb));

    return 2;
}

/**
 * Xor a with (hl)
 * 
 * 2 machine cycles
 */
static uint8_t xor_a_mhl(gb_t *gb, uint8_t opcode) {
    xor_helper(gb, mem_read_byte(gb, gb->cpu.hl));

    return 2;
}

/**
 * Compare helper for a and n
 */
static void cp_helper(gb_t *gb, uint8_t n) {
    gb->cpu.f = 0;
    gb->cpu.flag_n = 1;

    carry_check_sub(gb, gb->cpu.a, n);
    half_carry_check_sub(gb, gb->cpu.a, n);

    uint8_t result = gb->cpu.a - n;

    zero_check(gb, result);
}

/**
 * Compare a with r
 * 
 * 1 machine cycle
 */
static uint8_t cp_a_r(gb_t *gb, uint8_t opcode) {
    cp_helper(gb, *cpu_register_for_param(gb, OPCODE_PARAM_LOW(opcode)));

    return 1;
}

/**
 * Compare a with immediate
 * 
 * 2 machine cycles
 */
static uint8_t cp_a_n(gb_t *gb, uint8_t opcode) {
    cp_helper(gb, cpu_read_n(gb));

    return 2;
}

/**
 * Compare a with (hl)
 * 
 * 2 machine cycles
 */
static uint8_t cp_a_mhl(gb_t *gb, uint8_t opcode) {
    cp_helper(gb, mem_read_byte(gb, gb->cpu.hl));

    return 2;
}

/**
 * Increment r
 * 
 * 1 machine cycle
 */
static uint8_t inc_r(gb_t *gb, uint8_t opcode) {
    gb->cpu.flag_n = 0;

    uint8_t *r = cpu_register_for_param(gb, OPCODE_PARAM_HIGH(opcode));

    half_carry_check_add(gb, *r, 1);

    (*r)++;
    
    zero_check(gb, *r);

    return 1;
}

/**
 * Increment (hl)
 * 
 * 3 machine cycles
 */
static uint8_t inc_mhl(gb_t *gb, uint8_t opcode) {
    gb->cpu.flag_n = 0;

    uint8_t r = mem_read_byte(gb, gb->cpu.hl);

    half_carry_check_add(gb, r, 1);
    r++;
    zero_check(gb, r);

    mem_write_byte(gb, gb->cpu.hl, r);

    return 3;
}

/**
 * Decrement r
 * 
 * 1 machine cycle
 */
static uint8_t dec_r(gb_t *gb, uint8_t opcode) {
    gb->cpu.flag_n = 1;

    uint8_t *r = cpu_register_for_param(gb, OPCODE_PARAM_HIGH(opcode));

    half_carry_check_sub(gb, *r, 1);

    (*r)--;
   
    zero_check(gb, *r);
    return 1;
}

/**
 * Decrement (hl)
 * 
 * 3 machine cycles
 */
static uint8_t dec_mhl(gb_t *gb, uint8_t opcode) {
    gb->cpu.flag_n = 1;

    uint8_t r = mem_read_byte(gb, gb->cpu.hl);

    half_carry_check_sub(gb, r, 1);
    r--;
    zero_check(gb, r);
    
    mem_write_byte(gb, gb->cpu.hl, r);

    return 3;
}

/**
 * Decimal adjust a
 * 
 * 1 machine cycle
 */
static uint8_t daa(gb_t *gb, uint8_t opcode) {
    int16_t result = gb->cpu.a;

    if (gb->cpu.flag_n) {
        if (gb->cpu.flag_h) {
            result = (result - 0x06) & 0xFF;
        }

        if (gb->cpu.flag_c) {
            result -= 0x60;
        }
    }
    else {
        if ((gb->cpu.flag_h) || (result & 0x0F) > 0x09) {
            result += 0x06;
        }

        if ((gb->cpu.flag_c) || result > 0x9F) {
            result += 0x60;
        }
    }

    zero_check(gb, result & 0xFF);

    gb->cpu.flag_c = (result & 0x100) == 0x100;
    gb->cpu.flag_h = 0;

    gb->cpu.a = result;

    return 1;
}

/**
 * 1s compliment of a
 * 
 * 1 machine cycle
 */
static uint8_t cpl(gb_t *gb, uint8_t opcode) {
    gb->cpu.a ^= 0xFF;

    gb->cpu.flag_n = 1;
    gb->cpu.flag_h = 1;

    return 1;
}

// 16-bit ALU

/**
 * Add rr to hl
 * 
 * 2 machine cycles
 */
static uint8_t add_hl_rr(gb_t *gb, uint8_t opcode) {
    uint16_t *r;

    switch (OPCODE_PARAM_HIGH(opcode) >> 1) {
        case 0b00:
            r = &gb->cpu.bc;
            break;

        case 0b01:
            r = &gb->cpu.de;
            break;

        case 0b10:
            r = &gb->cpu.hl;
            break;

        case 0b11:
            r = &gb->cpu.sp;
            break;
    }

    gb->cpu.flag_n = 0;

    half_carry_check_add_word(gb, gb->cpu.hl, *r);
    carry_check_add_word(gb, gb->cpu.hl, *r);

    gb->cpu.hl += *r;

    return 2;
}

/**
 * Increment rr
 * 
 * 2 machine cycles
 */
static uint8_t inc_rr(gb_t *gb, uint8_t opcode) {
    switch (OPCODE_PARAM_HIGH(opcode) >> 1) {
        case 0b00:
            (gb->cpu.bc)++;
            break;

        case 0b01:
            (gb->cpu.de)++;
            break;

        case 0b10:
            (gb->cpu.hl)++;
            break;

        case 0b11:
            (gb->cpu.sp)++;
            break;
    }

    return 2;
}

/**
 * Decrement rr
 * 
 * 2 machine cycles
 */
static uint8_t dec_rr(gb_t *gb, uint8_t opcode) {
    switch (OPCODE_PARAM_HIGH(opcode) >> 1) {
        case 0b00:
            (gb->cpu.bc)--;
            break;

        case 0b01:
            (gb->cpu.de)--;
            break;

        case 0b10:
            (gb->cpu.hl)--;
            break;

        case 0b11:
            (gb->cpu.sp)--;
            break;
    }

    return 2;
}

/**
 * Add immediate signed to sp
 *
 * 4 machine cycles
 */
static uint8_t add_sp_e(gb_t *gb, uint8_t opcode) {
    gb->cpu.f = 0;

    int8_t e = cpu_read_e(gb);

    if (e >= 0) {
        carry_check_add_word(gb, gb->cpu.sp, e);
    } else {
        carry_check_sub_word(gb, gb->cpu.sp, -e);
    }

    // Half carry behaviour unknown, won't bother implementing for now

    gb->cpu.sp += e;

    return 4;
}

// Rotate and shift instructions

/**
 * Rotate left helper
 */
static uint8_t rlc_helper(gb_t *gb, uint8_t value) {
    gb->cpu.f = 0;
    uint16_t result = value << 1;

    if (!!(result & 0x100)) {
        gb->cpu.flag_c = 1;
        result |= 0x1;
    }

    result &= 0xFF;

    zero_check(gb, result);

    return result;
}

/**
 * Rotate left through carry helper
 */
static uint8_t rl_helper(gb_t *gb, uint8_t value) {
    uint8_t carry = gb->cpu.flag_c;
    uint16_t result = value << 1;

    gb->cpu.f = 0;
    gb->cpu.flag_c = !!(result & 0x100);

    result = (result & 0xFF) | carry;

    zero_check(gb, result);

    return result;
}

/**
 * Rotate right helper
 */
static uint8_t rrc_helper(gb_t *gb, uint8_t value) {
    gb->cpu.f = 0;

    uint8_t bit_zero = value & 0x1;
    uint8_t result = value >> 1;

    if (bit_zero) {
        gb->cpu.flag_c = 1;
        result |= 0x80;
    }

    zero_check(gb, result);

    return result;
}

/**
 * Rotate right through carry helper
 */
static uint8_t rr_helper(gb_t *gb, uint8_t value) {
    uint8_t bit_zero = value & 0x1;
    uint8_t carry = gb->cpu.flag_c;

    gb->cpu.f = 0;

    uint8_t result = value >> 1;

    gb->cpu.flag_c = bit_zero;

    result |= (carry << 7);

    zero_check(gb, result);

    return result;
}

/**
 * Rotate a left
 * 
 * 1 machine cycle
 */
static uint8_t rlca(gb_t *gb, uint8_t value) {
    gb->cpu.a = rlc_helper(gb, gb->cpu.a);
    gb->cpu.flag_z = 0;

    return 1;
}

/**
 * Rotate a left through carry
 * 
 * 1 machine cycle
 */
static uint8_t rla(gb_t *gb, uint8_t value) {
    gb->cpu.a = rl_helper(gb, gb->cpu.a);
    gb->cpu.flag_z = 0;

    return 1;
}

/**
 * Rotate a right
 * 
 * 1 machine cycle
 */
static uint8_t rrca(gb_t *gb, uint8_t value) {
    gb->cpu.a = rrc_helper(gb, gb->cpu.a);
    gb->cpu.flag_z = 0;

    return 1;
}

/**
 * Rotate a right through carry
 * 
 * 1 machine cycle
 */
static uint8_t rra(gb_t *gb, uint8_t opcode) {
    gb->cpu.a = rr_helper(gb, gb->cpu.a);
    gb->cpu.flag_z = 0;

    return 1;
}

/**
 * Rotate r left
 * 
 * 2 machine cycles
 */
static uint8_t rlc_r(gb_t *gb, uint8_t opcode) {
    uint8_t *r = cpu_register_for_param(gb, OPCODE_PARAM_LOW(opcode));
    *r = rlc_helper(gb, *r);

    return 2;
}

/**
 * Rotate r left through carry
 * 
 * 2 machine cycles
 */
static uint8_t rl_r(gb_t *gb, uint8_t opcode) {
    uint8_t *r = cpu_register_for_param(gb, OPCODE_PARAM_LOW(opcode));
    *r = rl_helper(gb, *r);

    return 2;
}

/**
 * Rotate r right
 * 
 * 2 machine cycles
 */
static uint8_t rrc_r(gb_t *gb, uint8_t opcode) {
    uint8_t *r = cpu_register_for_param(gb, OPCODE_PARAM_LOW(opcode));
    *r = rrc_helper(gb, *r);

    return 2;
}

/**
 * Rotate r right through carry
 * 
 * 2 machine cycles
 */
static uint8_t rr_r(gb_t *gb, uint8_t opcode) {
    uint8_t *r = cpu_register_for_param(gb, OPCODE_PARAM_LOW(opcode));

    *r = rr_helper(gb, *r);

    return 2;
}

/**
 * Rotate (hl) left
 * 
 * 4 machine cycles
 */
static uint8_t rlc_mhl(gb_t *gb, uint8_t opcode) {
    uint8_t r = mem_read_byte(gb, gb->cpu.hl);
    r = rlc_helper(gb, r);

    mem_write_byte(gb, gb->cpu.hl, r);

    return 4;
}

/**
 * Rotate (hl) left through carry
 * 
 * 4 machine cycles
 */
static uint8_t rl_mhl(gb_t *gb, uint8_t opcode) {
    uint8_t r = mem_read_byte(gb, gb->cpu.hl);
    r = rl_helper(gb, r);

    mem_write_byte(gb, gb->cpu.hl, r);

    return 4;
}

/**
 * Rotate (hl) right
 * 
 * 4 machine cycles
 */
static uint8_t rrc_mhl(gb_t *gb, uint8_t opcode) {
    uint8_t r = mem_read_byte(gb, gb->cpu.hl);
    r = rrc_helper(gb, r);

    mem_write_byte(gb, gb->cpu.hl, r);

    return 4;
}

/**
 * Rotate (hl) right through carry
 * 
 * 4 machine cycles
 */
static uint8_t rr_mhl(gb_t *gb, uint8_t opcode) {
    uint8_t r = mem_read_byte(gb, gb->cpu.hl);
    r = rr_helper(gb, r);

    mem_write_byte(gb, gb->cpu.hl, r);

    return 4;
}

/**
 * Shift left arithmetic r
 * 
 * 2 machine cycles
 */
static uint8_t sla_r(gb_t *gb, uint8_t opcode) {
    uint8_t *r = cpu_register_for_param(gb, OPCODE_PARAM_LOW(opcode));

    gb->cpu.f = 0;
    gb->cpu.flag_c = !!(*r & 0x80);

    *r = (*r << 1) & 0xFF;

    zero_check(gb, *r);

    return 2;
}

/**
 * Shift left arithmetic (hl)
 * 
 * 4 machine cycles
 */
static uint8_t sla_mhl(gb_t *gb, uint8_t opcode) {
    uint8_t r = mem_read_byte(gb, gb->cpu.hl);

    gb->cpu.f = 0;
    gb->cpu.flag_c = !!(r & 0x80);

    r = (r << 1) & 0xFF;

    zero_check(gb, r);

    mem_write_byte(gb, gb->cpu.hl, r);

    return 4;
}

/**
 * Shift right arithmetic r
 * 
 * 2 machine cycles
 */
static uint8_t sra_r(gb_t *gb, uint8_t opcode) {
    uint8_t *r = cpu_register_for_param(gb, OPCODE_PARAM_LOW(opcode));

    gb->cpu.f = 0;
    gb->cpu.flag_c = !!(*r & 0x1);

    *r = *r >> 1 | (*r & 0x80);

    zero_check(gb, *r);

    return 2;
}

/**
 * Shift right arithmetic (hl)
 * 
 * 4 machine cycles
 */
static uint8_t sra_mhl(gb_t *gb, uint8_t opcode) {
    uint8_t r = mem_read_byte(gb, gb->cpu.hl);

    gb->cpu.f = 0;
    gb->cpu.flag_c = !!(r & 0x1);

    r = r >> 1 | (r & 0x80);

    zero_check(gb, r);

    mem_write_byte(gb, gb->cpu.hl, r);

    return 4;
}

/**
 * Shift right logical r
 * 
 * 2 machine cycles
 */
static uint8_t srl_r(gb_t *gb, uint8_t opcode) {
    uint8_t *r = cpu_register_for_param(gb, OPCODE_PARAM_LOW(opcode));

    gb->cpu.f = 0;
    gb->cpu.flag_c = !!(*r & 0x1);

    *r = *r >> 1;

    zero_check(gb, *r);

    return 2;
}

/**
 * Shift right logical (hl)
 * 
 * 4 machine cycles
 */
static uint8_t srl_mhl(gb_t *gb, uint8_t opcode) {
    uint8_t r = mem_read_byte(gb, gb->cpu.hl);

    gb->cpu.f = 0;
    gb->cpu.flag_c = !!(r & 0x1);

    r = r >> 1;

    zero_check(gb, r);

    mem_write_byte(gb, gb->cpu.hl, r);

    return 4;
}

/**
 * Swap high and low nibbles of r
 * 
 * 2 machine cycles
 */
static uint8_t swap_r(gb_t *gb, uint8_t opcode) {
    uint8_t *r = cpu_register_for_param(gb, OPCODE_PARAM_LOW(opcode));
    *r = ((*r & 0xF) << 4) | ((*r & 0xF0) >> 4);

    return 2;
}

/**
 * Swap high and low nibbles of (hl)
 * 
 * 4 machine cycles
 */
static uint8_t swap_mhl(gb_t *gb, uint8_t opcode) {
    uint8_t r = mem_read_byte(gb, gb->cpu.hl);
    r = ((r & 0xF) << 4) | ((r & 0xF0) >> 4);

    mem_write_byte(gb, gb->cpu.hl, r);

    return 4;
}

// Bit operations

/**
 * Test bit n in r
 * 2 machine cycles
 */
static uint8_t bit_n_r(gb_t *gb, uint8_t opcode) {
    uint8_t *r = cpu_register_for_param(gb, OPCODE_PARAM_LOW(opcode));
    uint8_t b = OPCODE_PARAM_HIGH(opcode);

    gb->cpu.f = 0;
    gb->cpu.flag_h = 1;
    gb->cpu.flag_z = !(*r & (1 << b));

    return 2;
}

/**
 * Test bit n in (hl)
 * 3 machine cycles
 */
static uint8_t bit_n_mhl(gb_t *gb, uint8_t opcode) {
    uint8_t r = mem_read_byte(gb, gb->cpu.hl);
    uint8_t b = OPCODE_PARAM_HIGH(opcode);

    gb->cpu.f = 0;
    gb->cpu.flag_h = 1;
    gb->cpu.flag_z = !(r & (1 << b));

    return 3;
}

/**
 * Set bit n in r
 * 2 machine cycles
 */
static uint8_t set_n_r(gb_t *gb, uint8_t opcode) {
    uint8_t *r = cpu_register_for_param(gb, OPCODE_PARAM_LOW(opcode));
    uint8_t b = OPCODE_PARAM_HIGH(opcode);

    *r |= (1 << b);

    return 2;
}

/**
 * Set bit n in (hl)
 * 3 machine cycles
 */
static uint8_t set_n_mhl(gb_t *gb, uint8_t opcode) {
    uint8_t r = mem_read_byte(gb, gb->cpu.hl);
    uint8_t b = OPCODE_PARAM_HIGH(opcode);

    r |= (1 << b);
    
    mem_write_byte(gb, gb->cpu.hl, r);

    return 3;
}

/**
 * Reset bit n in r
 * 2 machine cycles
 */
static uint8_t res_n_r(gb_t *gb, uint8_t opcode) {
    uint8_t *r = cpu_register_for_param(gb, OPCODE_PARAM_LOW(opcode));
    uint8_t b = OPCODE_PARAM_HIGH(opcode);

    *r &= ~(1 << b);

    return 2;
}

/**
 * Reset bit n in (hl)
 * 3 machine cycles
 */
static uint8_t res_n_mhl(gb_t *gb, uint8_t opcode) {
    uint8_t r = mem_read_byte(gb, gb->cpu.hl);
    uint8_t b = OPCODE_PARAM_HIGH(opcode);

    r &= ~(1 << b);
    
    mem_write_byte(gb, gb->cpu.hl, r);

    return 3;
}

// CPU control instructions

/**
 * Compliment the carry flag
 * 
 * 1 machine cycle
 */
static uint8_t ccf(gb_t *gb, uint8_t opcode) {
    gb->cpu.flag_c ^= 1;

    return 1;
}

/**
 * Set the carry flag
 * 
 * 1 machine cycle
 */
static uint8_t scf(gb_t *gb, uint8_t opcode) {
    gb->cpu.flag_c = 1;

    return 1;
}

/**
 * No operation
 *
 * 1 machine cycle
 */
static uint8_t nop(gb_t *gb, uint8_t opcode) {
    return 1;
}

/**
 * Halt until interrupt
 */
static uint8_t halt(gb_t *gb, uint8_t opcode) {
    // TODO
    return 1;
}

/**
 * Stop
 */
static uint8_t stop(gb_t *gb, uint8_t opcode) {
    // TODO
    return 1;
}

/**
 * Disable interrupts
 * 
 * 1 machine cycle
 */
static uint8_t di(gb_t *gb, uint8_t opcode) {
    gb->ime = 0;

    return 1;
}

/**
 * Enabled interrupts
 *
 * 1 machine cycle
 */
static uint8_t ei(gb_t *gb, uint8_t opcode) {
    gb->ime = 1;

    return 1;
}

// Jump instructions

/**
 * Jump to address of immediate 16
 * 
 * 4 machine cycles
 */
static uint8_t jp_nn(gb_t *gb, uint8_t opcode) {
    gb->cpu.pc = cpu_read_nn(gb);

    return 4;
}

/**
 * Jump to address hl
 *
 * 1 machine cycle
 */
static uint8_t jp_hl(gb_t *gb, uint8_t opcode) {
    gb->cpu.pc = gb->cpu.hl;

    return 1;
}

/**
 * Jump to address of immediate 16 if cc is true
 * 
 * 4 / 3 machine cycles
 */
static uint8_t jpif_nn(gb_t *gb, uint8_t opcode) {
    uint8_t cc = cpu_cc_for_param(gb, OPCODE_PARAM_HIGH(opcode));
    uint16_t nn = cpu_read_nn(gb);

    if (cc) {
        gb->cpu.pc = nn;
    }

    return cc ? 4 : 3;
}

/**
 * Jump relative to pc by e
 * 
 * 3 machine cycles
 */
static uint8_t jr(gb_t *gb, uint8_t opcode) {
    int8_t e = cpu_read_e(gb);
    gb->cpu.pc += e;

    return 3;
}

/**
 * Jump relative to pc by e if cc is true
 * 
 * 3 / 2 machine cycles
 */
static uint8_t jrif(gb_t *gb, uint8_t opcode) {
    uint8_t cc = cpu_cc_for_param(gb, OPCODE_PARAM_HIGH(opcode) & 0b11);
    int8_t e = cpu_read_e(gb);

    if (cc) {
        gb->cpu.pc += e;
    }

    return cc ? 3 : 2;
}

/**
 * Call routine at nn
 *
 * 6 machine cycles
 */
static uint8_t call(gb_t *gb, uint8_t opcode) {
    uint16_t nn = cpu_read_nn(gb);

    gb->cpu.sp -= 2;
    mem_write_word(gb, gb->cpu.sp, gb->cpu.pc);

    gb->cpu.pc = nn;
    return 6;
}

/**
 * Call routine at nn if cc is true
 * 
 * 6 / 3 machine cycles
 */
static uint8_t callif(gb_t *gb, uint8_t opcode) {
    uint8_t cc = cpu_cc_for_param(gb, OPCODE_PARAM_HIGH(opcode));
    uint16_t nn = cpu_read_nn(gb);

    if (cc) {
        gb->cpu.sp -= 2;
        mem_write_word(gb, gb->cpu.sp, gb->cpu.pc);

        gb->cpu.pc = nn;
    }

    return cc ? 6 : 3;
}

/**
 * Return from routine
 * 
 * 4 machine cycles
 */
static uint8_t ret(gb_t *gb, uint8_t opcode) {
    gb->cpu.pc = mem_read_word(gb, gb->cpu.sp);
    gb->cpu.sp += 2;

    return 4;
}

/**
 * Return from routine if cc is true
 * 
 * 5 / 2 machine cycles
 */
static uint8_t retif(gb_t *gb, uint8_t opcode) {
    uint8_t cc = cpu_cc_for_param(gb, OPCODE_PARAM_HIGH(opcode));

    if (cc) {
        gb->cpu.pc = mem_read_word(gb, gb->cpu.sp);
        gb->cpu.sp += 2;
    }

    return cc ? 5 : 2;
}

/**
 * Return from interrupt
 * 
 * 4 machine cycles
 */
static uint8_t reti(gb_t *gb, uint8_t opcode) {
    gb->cpu.pc = mem_read_word(gb, gb->cpu.sp);
    gb->cpu.sp += 2;

    gb->ime = 1;

    return 4;
}

/**
 * Reset to position specified by n
 * 
 * 4 machine cycles
 */
static uint8_t rst(gb_t *gb, uint8_t opcode) {
    uint8_t n = OPCODE_PARAM_HIGH(opcode);

    gb->cpu.sp -= 2;
    mem_write_word(gb, gb->cpu.sp, gb->cpu.pc);

    gb->cpu.pc = ((n & 0b110) << 3) | ((n & 1) << 3);

    return 4;
}

/**
 * CB mapping function
 */
static uint8_t cb_map(gb_t *gb, uint8_t opcode) {
    uint8_t new_opcode = cpu_read_program(gb);

    #if OPCODE_DEBUG
        #if !OPCODE_BIOS_DEBUG
            if (!gb->in_bios)
        #endif
        if (gb->cpu.pc < 0x02ED || gb->cpu.pc > 0x02F1) printf("%X\t", new_opcode);
    #endif
    
    switch (new_opcode >> 3) {
        case 0:
            return ((new_opcode & 0xF) == 0xE) || ((new_opcode & 0xF) == 0x6) ? rlc_mhl(gb, new_opcode) : rlc_r(gb, new_opcode);

        case 1:
            return ((new_opcode & 0xF) == 0xE) || ((new_opcode & 0xF) == 0x6) ? rrc_mhl(gb, new_opcode) : rrc_r(gb, new_opcode);

        case 2:
            return ((new_opcode & 0xF) == 0xE) || ((new_opcode & 0xF) == 0x6) ? rl_mhl(gb, new_opcode) : rl_r(gb, new_opcode);

        case 3:
            return ((new_opcode & 0xF) == 0xE) || ((new_opcode & 0xF) == 0x6) ? rr_mhl(gb, new_opcode) : rr_r(gb, new_opcode);

        case 4:
            return ((new_opcode & 0xF) == 0xE) || ((new_opcode & 0xF) == 0x6) ? sla_mhl(gb, new_opcode) : sla_r(gb, new_opcode);

        case 5:
            return ((new_opcode & 0xF) == 0xE) || ((new_opcode & 0xF) == 0x6) ? sra_mhl(gb, new_opcode) : sra_r(gb, new_opcode);

        case 6:
            return ((new_opcode & 0xF) == 0xE) || ((new_opcode & 0xF) == 0x6) ? swap_mhl(gb, new_opcode) : swap_r(gb, new_opcode);

        case 7:
            return ((new_opcode & 0xF) == 0xE) || ((new_opcode & 0xF) == 0x6) ? srl_mhl(gb, new_opcode) : srl_r(gb, new_opcode);

        default:
            switch (new_opcode >> 6) {
                case 1:
                    return ((new_opcode & 0xF) == 0xE) || ((new_opcode & 0xF) == 0x6) ? bit_n_mhl(gb, new_opcode) : bit_n_r(gb, new_opcode);
                
                case 2:
                    return ((new_opcode & 0xF) == 0xE) || ((new_opcode & 0xF) == 0x6) ? res_n_mhl(gb, new_opcode) : res_n_r(gb, new_opcode);

                default:
                    return ((new_opcode & 0xF) == 0xE) || ((new_opcode & 0xF) == 0x6) ? set_n_mhl(gb, new_opcode) : set_n_r(gb, new_opcode);
            }
    }
}

/**
 * No opcode for this value. Crash the program
 */
static uint8_t no_opcode(gb_t *gb, uint8_t opcode) {
    printf("Opcode undefined - illegal");
    abort();
}

static cpu_instruction_t* const cpu_opcode_table[] = {
/*          0x-0        0x-1        0x-2        0x-3        0x-4        0x-5        0x-6        0x-7        0x-8        0x-9        0x-A        0x-B        0x-C        0x-D        0x-E        0x-F */
/* 0x0- */  nop,        ld_rr_nn,   ld_mbc_a,   inc_rr,     inc_r,      dec_r,      ld_r_n,     rlca,       ld_mnn_sp,  add_hl_rr,  ld_a_mbc,   dec_rr,     inc_r,      dec_r,      ld_r_n,     rrca,
/* 0x1- */  stop,       ld_rr_nn,   ld_mde_a,   inc_rr,     inc_r,      dec_r,      ld_r_n,     rla,        jr,         add_hl_rr,  ld_a_mde,   dec_rr,     inc_r,      dec_r,      ld_r_n,     rra,
/* 0x2- */  jrif,       ld_rr_nn,   ldi_mhl_a,  inc_rr,     inc_r,      dec_r,      ld_r_n,     daa,        jrif,       add_hl_rr,  ldi_a_mhl,  dec_rr,     inc_r,      dec_r,      ld_r_n,     cpl,
/* 0x3- */  jrif,       ld_rr_nn,   ldd_mhl_a,  inc_rr,     inc_mhl,    dec_mhl,    ld_mhl_n,   scf,        jrif,       add_hl_rr,  ldd_a_mhl,  dec_rr,     inc_r,      dec_r,      ld_r_n,     ccf,
/* 0x4- */  ld_r_r,     ld_r_r,     ld_r_r,     ld_r_r,     ld_r_r,     ld_r_r,     ld_r_mhl,   ld_r_r,     ld_r_r,     ld_r_r,     ld_r_r,     ld_r_r,     ld_r_r,     ld_r_r,     ld_r_mhl,   ld_r_r,
/* 0x5- */  ld_r_r,     ld_r_r,     ld_r_r,     ld_r_r,     ld_r_r,     ld_r_r,     ld_r_mhl,   ld_r_r,     ld_r_r,     ld_r_r,     ld_r_r,     ld_r_r,     ld_r_r,     ld_r_r,     ld_r_mhl,   ld_r_r,
/* 0x6- */  ld_r_r,     ld_r_r,     ld_r_r,     ld_r_r,     ld_r_r,     ld_r_r,     ld_r_mhl,   ld_r_r,     ld_r_r,     ld_r_r,     ld_r_r,     ld_r_r,     ld_r_r,     ld_r_r,     ld_r_mhl,   ld_r_r,
/* 0x7- */  ld_mhl_r,   ld_mhl_r,   ld_mhl_r,   ld_mhl_r,   ld_mhl_r,   ld_mhl_r,   halt,       ld_mhl_r,   ld_r_r,     ld_r_r,     ld_r_r,     ld_r_r,     ld_r_r,     ld_r_r,     ld_r_mhl,   ld_r_r,
/* 0x8- */  add_a_r,    add_a_r,    add_a_r,    add_a_r,    add_a_r,    add_a_r,    add_a_mhl,  add_a_r,    adc_a_r,    adc_a_r,    adc_a_r,    adc_a_r,    adc_a_r,    adc_a_r,    adc_a_mhl,  adc_a_r,
/* 0x9- */  sub_a_r,    sub_a_r,    sub_a_r,    sub_a_r,    sub_a_r,    sub_a_r,    sub_a_mhl,  sub_a_r,    sbc_a_r,    sbc_a_r,    sbc_a_r,    sbc_a_r,    sbc_a_r,    sbc_a_r,    sbc_a_mhl,  sbc_a_r,
/* 0xA- */  and_a_r,    and_a_r,    and_a_r,    and_a_r,    and_a_r,    and_a_r,    and_a_mhl,  and_a_r,    xor_a_r,    xor_a_r,    xor_a_r,    xor_a_r,    xor_a_r,    xor_a_r,    xor_a_mhl,  xor_a_r,
/* 0xB- */  or_a_r,     or_a_r,     or_a_r,     or_a_r,     or_a_r,     or_a_r,     or_a_mhl,   or_a_r,     cp_a_r,     cp_a_r,     cp_a_r,     cp_a_r,     cp_a_r,     cp_a_r,     cp_a_mhl,   cp_a_r,
/* 0xC- */  retif,      pop_rr,     jpif_nn,    jp_nn,      callif,     push_rr,    add_a_n,    rst,        retif,      ret,        jpif_nn,    cb_map,     callif,     call,       adc_a_n,    rst,
/* 0xD- */  retif,      pop_rr,     jpif_nn,    no_opcode,  callif,     push_rr,    sub_a_n,    rst,        retif,      reti,       jpif_nn,    no_opcode,  callif,     no_opcode,  sbc_a_n,    rst,
/* 0xE- */  ldh_mn_a,   pop_rr,     ldh_mc_a,   no_opcode,  no_opcode,  push_rr,    and_a_n,    rst,        add_sp_e,   jp_hl,      ld_mnn_a,   no_opcode,  no_opcode,  no_opcode,  xor_a_n,    rst,
/* 0xF- */  ldh_a_mn,   pop_rr,     ldh_a_mc,   di,         no_opcode,  push_rr,    or_a_n,     rst,        ld_hl_spe,  ld_sp_hl,   ld_a_mnn,   ei,         no_opcode,  no_opcode,  cp_a_n,     rst,
};

#define INTERRUPT_VBLANK 0x0040
#define INTERRUPT_LCD_STATUS 0x0048
#define INTERRUPT_TIMER 0x0050
#define INTERRUPT_SERIAL 0x0058
#define INTERRUPT_JOYPAD 0x0060

/**
 * Call an interrupt
 */
uint8_t call_interrupt(gb_t *gb, uint16_t addr) {
    gb->ime = 0;

    gb->cpu.sp -= 2;
    mem_write_word(gb, gb->cpu.sp, gb->cpu.pc);

    gb->cpu.pc = addr;

    // 3 machine cycles
    return 3;
}

/**
 * Initialise the CPU by setting the registers to 0
 * and the SP to 0xFFFE
 */
void cpu_init(gb_t *gb) {
    gb->cpu.a = 0;
    gb->cpu.b = 0;
    gb->cpu.c = 0;
    gb->cpu.d = 0;
    gb->cpu.e = 0;
    gb->cpu.f = 0;
    gb->cpu.l = 0;
    gb->cpu.h = 0;

    gb->cpu.sp = 0xFFFE;
    gb->cpu.pc = 0x0;

    gb->cpu.remaining_machine_cycles = 0;

    gb->ime = 1;
}

/**
 * Read, decode, execute loop
 */
void cpu_tick(gb_t *gb) {
    if (gb->cpu.remaining_machine_cycles == 0) {
        // Ready for next instruction
        // Otherwise, theoretically doing a previous instruction, so wait
        
        // Opcode
        uint8_t opcode = cpu_read_program(gb);

        #if OPCODE_DEBUG
            #if !OPCODE_BIOS_DEBUG
                if (!gb->in_bios)
            #endif
            if (gb->cpu.pc < 0x02ED || gb->cpu.pc > 0x02F1) printf("%04X\t%X", gb->cpu.pc - 1, opcode);
        #endif

        gb->cpu.remaining_machine_cycles += cpu_opcode_table[opcode](gb, opcode);

        #if OPCODE_DEBUG
            #if !OPCODE_BIOS_DEBUG
                if (!gb->in_bios)
            #endif
            if (gb->cpu.pc < 0x02ED || gb->cpu.pc > 0x02F1) printf("\n");
        #endif

        // Check for interrupts
        if (gb->ime) {
            uint8_t interrupts_enabled = mem_read_byte(gb, INTERRUPT_ENABLE);
            uint8_t interrupt_flags = mem_read_byte(gb, INTERRUPT_FLAGS);
            uint8_t interrupts_fired_masked = interrupts_enabled & interrupt_flags;

            if (interrupts_fired_masked & INT_FLAG_VBLANK) {
                // Vblank
                gb->cpu.remaining_machine_cycles += call_interrupt(gb, INTERRUPT_VBLANK);
                mem_write_byte(gb, INTERRUPT_FLAGS, interrupt_flags & ~INT_FLAG_VBLANK);
            }

            if (interrupts_fired_masked & INT_FLAG_LCD) {
                // LCD status
                mem_write_byte(gb, INTERRUPT_FLAGS, interrupt_flags & ~INT_FLAG_LCD);
                gb->cpu.remaining_machine_cycles += call_interrupt(gb, INTERRUPT_LCD_STATUS);
            }

            if (interrupts_fired_masked & INT_FLAG_TIMER) {
                // Timer
                mem_write_byte(gb, INTERRUPT_FLAGS, interrupt_flags & ~INT_FLAG_TIMER);
                gb->cpu.remaining_machine_cycles += call_interrupt(gb, INTERRUPT_TIMER);
            }

            if (interrupts_fired_masked & INT_FLAG_SERIAL) {
                // Serial
                mem_write_byte(gb, INTERRUPT_FLAGS, interrupt_flags & ~INT_FLAG_SERIAL);
                gb->cpu.remaining_machine_cycles += call_interrupt(gb, INTERRUPT_SERIAL);
            }

            if (interrupts_fired_masked & INT_FLAG_JOYPAD) {
                // Joypad
                mem_write_byte(gb, INTERRUPT_FLAGS, interrupt_flags & ~INT_FLAG_JOYPAD);
                gb->cpu.remaining_machine_cycles += call_interrupt(gb, INTERRUPT_JOYPAD);
            }
        }

        if (gb->cpu.pc > 0xFF && TEST_BIOS) {
            printf("A: %02X\tF: %02X\n", gb->cpu.a, gb->cpu.f);
            printf("B: %02X\tC: %02X\n", gb->cpu.b, gb->cpu.c);
            printf("D: %02X\tE: %02X\n", gb->cpu.d, gb->cpu.e);
            printf("H: %02X\tL: %02X\n", gb->cpu.h, gb->cpu.l);
            printf("AF: %04X\tBC: %04X\tDE: %04X\tHL: %04X\n", gb->cpu.af, gb->cpu.bc, gb->cpu.de, gb->cpu.hl);
            printf("SP: %04X\tPC: %04X\n", gb->cpu.sp, gb->cpu.pc);
            printf("0xFF80: %02X, 0xFF81: %02X\n", mem_read_byte(gb, 0xFF80), mem_read_byte(gb, 0xFF81));
            printf("Z: %i\tN: %i\tH: %i\tC: %i\n", gb->cpu.flag_z, gb->cpu.flag_n, gb->cpu.flag_h, gb->cpu.flag_c);
            abort();
        }
    }

    gb->cpu.remaining_machine_cycles--;
}