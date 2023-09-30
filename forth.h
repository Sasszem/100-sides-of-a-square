#pragma once
#include <stdint.h>


#define DN_ADDRESS_LEN 18
#define CELL 4
#define RETURN_STACK_SIZE (256*CELL)

/**
    HEADER STRUCTURE
    1b N6 N5 N4 N3 N2 N1 N0    AH AS AI AP ?? ?? LL LL    LL LL LL LL LL LL L9 L8    L7 L6 L5 L4 L3 L2 L1 L0
    31 30 29 28 27 26 25 24    23 22 21 20 19 18 17 16    15 14 13 12 11 10 09 08    07 06 05 04 03 02 01 00
*/
#define _WORD_IS_PRIMITIVE  (1<<20)
#define _WORD_IS_IMMEDIATE  (1<<21)
#define _WORD_IS_SKIP       (1<<22)
#define _WORD_IS_HIDDEN     (1<<23)
typedef int64_t T_IMMEDIATE;

typedef uint32_t dn_cell_t;
typedef void (*dn_word_callback)(void);

extern uint8_t DN_FORTH_MEMORY[1<<DN_ADDRESS_LEN];







void dn_init();


/////////////////
// WORDS group //
/////////////////

/**
    Test if an address points to a word or not.
    Might give false positives!
*/
int dn_is_word(dn_cell_t addr);

/**
Try looking up a word. Return it if success, 0 if not found
*/
dn_cell_t dn_lookup_word(const char* name);
dn_cell_t dn_lookup_word_counted(const char* name, dn_cell_t len);
dn_cell_t dn_lookup_word_containing_address(dn_cell_t search);
dn_cell_t dn_next_word_after_address(dn_cell_t search);

/**
Create empty word with NAME. 
*/
void dn_create_word(const char* name);
void dn_finish_compiled_word();

void dn_set_cfa(dn_cell_t word_start, dn_cell_t value);
void dn_set_pfa(dn_cell_t word_start, dn_cell_t value);

uint8_t dn_word_get_name_len(dn_cell_t word);
const char* dn_word_get_name_ptr(dn_cell_t word);

// warning: does not check if XT is indeed a primitive, must be checked manually!
dn_word_callback dn_word_get_fptr_from_xt(dn_cell_t xt);
dn_cell_t dn_word_get_prev_ptr_from_word(dn_cell_t word);

void dn_word_set_immediate();
void dn_word_set_hidden();

///////////////////
// MEMORY group //
///////////////////


#define _HERE_HEADER (RETURN_STACK_SIZE)
// 3 cells for header
// 2 cells for name
#define _HERE_VALUE_LOCATION (_HERE_HEADER + 4*CELL)
#define _HERE_START_VALUE (_HERE_VALUE_LOCATION)

// 2 cells for constant working
#define _DP_HEADER (_HERE_VALUE_LOCATION + CELL)
#define _DP_VALUE_LOCATION (_DP_HEADER + 4*CELL)
#define _DP_START_VALUE (_DP_HEADER)
#define _SP_VALUE_LOCATION (_DP_VALUE_LOCATION + 5*CELL)


dn_cell_t dn_mask_address(dn_cell_t ptr);
dn_cell_t dn_mask_cell_address(dn_cell_t ptr);

/**
Read a character from memory. Masks unused address bits.
*/
uint8_t dn_read_char(dn_cell_t address);

/**
Read a cell from memory. Masks unused address bits, aut-aligns to cell beginning.
*/
dn_cell_t dn_read_cell(dn_cell_t address);

void dn_write_char(dn_cell_t address, uint8_t chr);
void dn_write_cell(dn_cell_t address, dn_cell_t cell);

////////////////////
// MEMORY/GLOBALS //
////////////////////
/**
get value of HERE ptr. Uses internal address of variable
*/
dn_cell_t dn_get_here();
void dn_set_here(dn_cell_t ptr);
void dn_align_here();

void dn_set_dp(dn_cell_t word);
dn_cell_t dn_get_dp();

void dn_set_sp(dn_cell_t sp);
dn_cell_t dn_get_sp();




dn_cell_t dn_allot(dn_cell_t size);
void dn_allot_str(const char* str);
dn_cell_t dn_allot_write(dn_cell_t value);
void dn_allot_counted(const char* str);

//////////////////
// MEMORY/STACK //
//////////////////


void dn_push(dn_cell_t value);
dn_cell_t dn_pop();
dn_cell_t dn_peek(dn_cell_t idx);

void dn_rpush(dn_cell_t value);
dn_cell_t dn_rpop();
dn_cell_t dn_rpeek(dn_cell_t idx);

dn_cell_t dn_get_var(const char* name);
void dn_set_var(const char* name, dn_cell_t value);

///////////////////
// COMPILE group //
///////////////////

void dn_create_system_word(const char* name, dn_word_callback fun);
void dn_create_variable(const char* name, dn_cell_t value);
dn_cell_t dn_start_word(const char* name);
void dn_compile_literal(dn_cell_t literal);
void dn_compile(const char* name);

/////////////////
// DEBUG group //
/////////////////

void dn_dump_memory(dn_cell_t start_address, dn_cell_t len);
void dn_dump_cells(dn_cell_t start_address, dn_cell_t len);
void dn_disas_word(dn_cell_t xt);
void dn_debug_dump_stack();
void dn_debug_dump_ret_stack();
void dn_print_backtrace();

///////////////////
// EXECUTE group //
///////////////////
void dn_execute(const char* name);
void dn_execute_token(dn_cell_t xt);
void dn_abort(const char* msg);