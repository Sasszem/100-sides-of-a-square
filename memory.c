#include "forth.h"
#include <stdio.h>

uint8_t DN_FORTH_MEMORY[1<<DN_ADDRESS_LEN];

dn_cell_t dn_mask_address(dn_cell_t ptr) {
    return ptr & ((1<<DN_ADDRESS_LEN)-1);
}

dn_cell_t dn_mask_cell_address(dn_cell_t ptr) {
    return dn_mask_address(ptr) - (ptr % CELL); // remove last 2 bits
}


uint8_t dn_read_char(dn_cell_t address) {
    return DN_FORTH_MEMORY[dn_mask_address(address)];
}
void dn_write_char(dn_cell_t address, uint8_t chr) {
    DN_FORTH_MEMORY[dn_mask_address(address)] = chr;
}

dn_cell_t dn_read_cell(dn_cell_t address) {
    dn_cell_t cell_start_address = dn_mask_cell_address(address);
    return ((dn_cell_t)(DN_FORTH_MEMORY[cell_start_address  ]) << 24) +
           ((dn_cell_t)(DN_FORTH_MEMORY[cell_start_address+1]) << 16) +
           ((dn_cell_t)(DN_FORTH_MEMORY[cell_start_address+2]) << 8) +
           ((dn_cell_t)(DN_FORTH_MEMORY[cell_start_address+3]) );
}

void dn_write_cell(dn_cell_t address, dn_cell_t cell) {
    dn_cell_t cell_start_address = dn_mask_cell_address(address);
    DN_FORTH_MEMORY[cell_start_address + 0] = (cell>>24) & 0xff;
    DN_FORTH_MEMORY[cell_start_address + 1] = (cell>>16) & 0xff;
    DN_FORTH_MEMORY[cell_start_address + 2] = (cell>> 8) & 0xff;
    DN_FORTH_MEMORY[cell_start_address + 3] = (cell>> 0) & 0xff;
}


void dn_push(dn_cell_t value) {
    dn_cell_t sp = dn_get_sp(); 
    dn_write_cell(sp, value);
    dn_set_sp(sp - CELL);
}

dn_cell_t dn_pop() {
    dn_cell_t sp = dn_get_sp();
    if (sp == dn_get_var("S0")) {
        dn_abort("STACK UNDERFLOW");
        return 0;
    }
    dn_set_sp(sp + CELL);
    return dn_read_cell(sp + CELL);
}

dn_cell_t dn_peek(dn_cell_t idx) {
    dn_cell_t sp = dn_get_var("SP@");
    dn_cell_t depth = (dn_get_var("S0") - sp) / CELL;
    if (depth<=idx) {
        dn_abort("STACK UNDERFLOW");
        return 0;
    }
    return dn_read_cell(sp + CELL + idx * CELL);
}


dn_cell_t dn_rpeek(dn_cell_t idx) {
    dn_cell_t sp = dn_get_var("RP@");
    dn_cell_t depth = (dn_get_var("R0") - sp) / CELL;
    if (depth<idx) {
        dn_abort("CONTROL STACK UNDERFLOW");
        return 0;
    }
    return dn_read_cell(sp + CELL + idx * CELL);
}

dn_cell_t dn_rpop() {
    dn_cell_t sp = dn_get_var("RP@");
    if (sp == dn_get_var("R0")) {
        dn_abort("CONTROL STACK UNDERFLOW");
        return 0;
    }
    dn_set_var("RP@", sp + CELL);
    return dn_read_cell(sp + CELL);
    return 0;
}

void dn_rpush(dn_cell_t value) {
    // return 0 if empty

    dn_cell_t rp = dn_get_var("RP@"); 
    dn_write_cell(rp, value);
    dn_set_var("RP@", rp - CELL);
}

dn_cell_t dn_get_dp() {
    return dn_read_cell(_DP_VALUE_LOCATION);
}

void dn_set_dp(dn_cell_t word) {
    dn_write_cell(_DP_VALUE_LOCATION, word);
}


dn_cell_t dn_get_sp() {
    return dn_read_cell(_SP_VALUE_LOCATION);
}

void dn_set_sp(dn_cell_t sp) {
    dn_write_cell(_SP_VALUE_LOCATION, sp);
}

void dn_align_here() {
    uint8_t val = 4 - (dn_get_here() & 3);
    val = val % 4;
    dn_set_here(dn_get_here() + val);
}

dn_cell_t dn_get_here() {
    return dn_read_cell(_HERE_VALUE_LOCATION);
}

void dn_set_here(dn_cell_t value) {
    dn_write_cell(_HERE_VALUE_LOCATION, value);
}


void dn_set_var(const char* name, dn_cell_t value) {
    dn_cell_t xt = dn_lookup_word(name);
    if (xt) {
        dn_write_cell(dn_read_cell(xt + CELL), value);
    } else {
        printf("Variable '%s' not found!\n", name);
    }
}


dn_cell_t dn_get_var(const char* name) {
    dn_cell_t xt = dn_lookup_word(name);
    if (xt) {
        return dn_read_cell(dn_read_cell(xt + CELL));
    } else {
        printf("Variable '%s' not found!\n", name);
    }
    return 0;
}