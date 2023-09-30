#include "forth_words.h"

dn_cell_t _get_immediate_val() {
    dn_cell_t ptr = dn_get_var("[IP]");
    dn_set_var("[IP]", ptr + CELL);
    // printf("LITERAL was called: %d\n", dn_read_cell(ptr));
    return dn_read_cell(ptr);
}