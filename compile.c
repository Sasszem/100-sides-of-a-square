#include "forth.h"
#include <stdio.h>

// WARNING: HERE IS NOT ALWAYS ALIGNED!
dn_cell_t dn_allot(dn_cell_t size) {
    dn_cell_t old_here = dn_get_here();
    dn_set_here(old_here + size);
    return old_here;
}

dn_cell_t dn_allot_write(dn_cell_t value) {
    dn_align_here();
    dn_cell_t location = dn_get_here();
    dn_write_cell(location, value);
    dn_allot(CELL);
    return location;
}

void dn_allot_str(const char* str) {
    while (*str) {
        dn_write_char(dn_get_here(), *str);
        dn_allot(1);
        str++;
    }
}

void dn_allot_counted(const char* str) {
    dn_cell_t base = dn_get_here();
    dn_allot(1);
    while (*str) {
        dn_write_char(dn_get_here(), *str);
        dn_allot(1);
        dn_write_char(base, dn_read_char(base)+1);
        str++;
    }
}


void dn_compile(const char* name) {
    dn_cell_t token = dn_lookup_word(name);
    if (!token) {
        printf("error! Could not compile '%s' in '%.*s'!\n", name, dn_word_get_name_len(dn_get_dp()), dn_word_get_name_ptr(dn_get_dp()));
    }
    dn_allot_write(token);
}


void dn_compile_literal(dn_cell_t literal) {
    dn_compile("[LITERAL]");
    dn_allot_write(literal);
}


