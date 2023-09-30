#include "forth.h"
#include <stddef.h>
#include <stdio.h>
#include <string.h>

static void print_word_name(dn_cell_t addr);
static int min(int a, int b) {
    return a<b ? a : b;
} 


int dn_is_word(dn_cell_t addr) {
    // check if the bit is set...
    return !!(dn_read_cell(addr) & (1<<31));
}


void dn_create_word(const char *name) {
    // init with 0 CFA and PFA, no flagz, etc.
    uint8_t name_len = strlen(name);

    dn_align_here();
    dn_cell_t word_loc = dn_get_here();
    
    dn_allot_write(dn_mask_cell_address(dn_get_dp()) | ((dn_cell_t)(name_len & 0x7f) << 24) | (1<<31));
    dn_allot_write(0); // for CFA
    dn_allot_write(0); // for PFA
    dn_allot_str(name);
    dn_set_dp(word_loc);
    // printf("Created word '%s' at %06x, %06x\n", name, word_loc, dn_get_dp());
}

void dn_set_pfa(dn_cell_t word_start, dn_cell_t value) {
    // printf("Writing PFA of word startin at %06x (address %06x) to %06x\n", word_start, word_start+CELL, value);
    dn_write_cell(word_start + 1*CELL, value);
}

void dn_set_cfa(dn_cell_t word_start, dn_cell_t value) {
    dn_write_cell(word_start + 2*CELL, value);
}

void dn_word_set_immediate() {
    dn_write_cell(dn_get_dp(), dn_read_cell(dn_get_dp()) | _WORD_IS_IMMEDIATE);
}

void dn_word_set_hidden() {
    dn_write_cell(dn_get_dp(), dn_read_cell(dn_get_dp()) | _WORD_IS_HIDDEN);
}

void dn_create_variable(const char* name, dn_cell_t value) {
    // create word
    dn_create_word(name);

    // printf("Freshly created word:\n");
    // dn_dump_cells(0, 20);
    // printf("\n\n");
    
    // allot content
    dn_cell_t pfa = dn_allot_write(value);
    // set PFA
    dn_set_pfa(dn_get_dp(), pfa);
    // set CFA
    // printf("Set up PFA & CFA:\n");
    // dn_dump_cells(0, 20);
    // printf("\n\n");
}


void dn_create_system_word(const char *name, dn_word_callback fun) {
    dn_create_word(name);
    dn_write_cell(dn_get_dp(), dn_read_cell(dn_get_dp()) | _WORD_IS_PRIMITIVE);
    dn_set_pfa(dn_get_dp(), ((uint64_t)fun >> 32) & 0xffffffff);
    dn_set_cfa(dn_get_dp(), (uint64_t)fun & 0xffffffff);
}

dn_cell_t dn_word_get_prev_ptr_from_word(dn_cell_t word) {
    return dn_mask_cell_address(dn_read_cell(word));
}


dn_cell_t dn_lookup_word_counted(const char* name, dn_cell_t len) {
    // start at DP
    // printf("Looking up '%s'\n", name);
    dn_cell_t addr = dn_get_dp();
    do {
        // printf("Word at %x: '%.*s'\n", addr, dn_word_get_name_len(addr), dn_word_get_name_ptr(addr));
        if (!strncasecmp(name, dn_word_get_name_ptr(addr), min(len,dn_word_get_name_len(addr))) && dn_word_get_name_len(addr) == len) {
            if (dn_read_cell(addr) & _WORD_IS_SKIP) {
                // printf("Word '%.*s' is SKIP! (header: %x)\n", dn_word_get_name_len(addr), dn_word_get_name_ptr(addr), dn_read_cell(addr));
            } else {
                return addr;
            }
        }

        addr = dn_word_get_prev_ptr_from_word(addr);
    } while (addr);
    // printf("Warning: word '%s' not found!\n", name);
    return 0;
}

dn_cell_t dn_lookup_word(const char *name) {
    return dn_lookup_word_counted(name, strlen(name));
}


dn_cell_t dn_start_word(const char* name) {
    dn_create_word(name);
    dn_align_here();
    dn_set_pfa(dn_get_dp(), dn_get_here());
    dn_set_cfa(dn_get_dp(), dn_get_here());
    dn_write_cell(dn_get_dp(), dn_read_cell(dn_get_dp()) | _WORD_IS_SKIP);
    dn_compile("DROP");
    return dn_get_here();
}

void dn_finish_compiled_word() {
    dn_compile("EXIT");
    // printf("Finishing word \n");
    // print_word_name(dn_get_dp());
    // printf("\n");
    dn_write_cell(dn_get_dp(), dn_read_cell(dn_get_dp()) & ~(_WORD_IS_SKIP));
}

uint8_t dn_word_get_name_len(dn_cell_t word) {
    if (!dn_is_word(word))
        return 0;
    return (dn_read_cell(word) >> 24) & 0x7f;
}

const char* dn_word_get_name_ptr(dn_cell_t word) {
    if (!dn_is_word(word))
        return 0;
    return (const char*)&DN_FORTH_MEMORY[word + 3*CELL];
}


dn_word_callback dn_word_get_fptr_from_xt(dn_cell_t xt) {
    return (dn_word_callback)(((uint64_t)dn_read_cell(xt+CELL) << 32) | (uint64_t)dn_read_cell(xt+2*CELL));
}



dn_cell_t dn_lookup_word_containing_address(dn_cell_t search) {
    dn_cell_t addr = dn_get_dp();
    dn_cell_t next_addr = dn_get_dp();
    if (search >= addr) {
        return addr;
    }

    do {
        next_addr = dn_word_get_prev_ptr_from_word(addr);
        if (addr > search && search >= next_addr)
            return next_addr;
        addr = next_addr;
    } while (addr);
    return 0;
}

// return ptr to xt of next word
// if last word or later, return here ptr
// if before first word, return location of HERE (1st word)
dn_cell_t dn_next_word_after_address(dn_cell_t search) {
    dn_cell_t addr = dn_get_dp();
    dn_cell_t next_addr = dn_get_dp();
    if (search >= addr) {
        return dn_get_here();
    }

    do {
        next_addr = dn_word_get_prev_ptr_from_word(addr);
        if (addr > search && search >= next_addr)
            return addr;
        addr = next_addr;
    } while (addr);
    return _HERE_HEADER;
}


void print_word_name(dn_cell_t addr) {
    printf("%.*s", dn_word_get_name_len(addr), dn_word_get_name_ptr(addr));
}
