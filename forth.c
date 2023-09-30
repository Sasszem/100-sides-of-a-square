#include "forth.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "simple_compile.h"


#include "words/forth_words.h"

void dn_init() {
    dn_set_here(_HERE_HEADER);
    dn_set_dp(0);
    dn_create_variable("HERE", _HERE_START_VALUE);

    dn_create_variable("DP", _DP_START_VALUE);


    dn_create_variable("SP@", 0);
    dn_create_variable("S0", dn_mask_cell_address(1<<16));
    dn_set_var("SP@", dn_get_var("S0"));


    dn_create_variable("R0", _HERE_HEADER - CELL);
    dn_create_variable("RP@",0);
    dn_set_var("RP@", dn_get_var("R0"));

    dn_create_variable("[IP]", 0);
    dn_word_set_hidden();

    dn_create_variable("[XT]", 0);
    dn_word_set_hidden();

    dn_create_variable("[ABORT?]", 0);
    dn_word_set_hidden();

    words_set_core();
    words_set_stack_memory();
    words_set_arith();
    words_set_io();



    words_set_derived_stack_memory();
    words_set_control();
    words_set_derived_arith();
    words_set_derived_io();
    words_set_core_aux();
    words_set_derived_2_stack_memory();

    words_set_words();
    
    words_set_control_2();
    words_set_aux();

    // set HERE, RP@, SP@ as constants...
    // code for them is stored at the definition of HERE
    dn_cell_t cfa = dn_read_cell(dn_lookup_word("CONSTANT")+2*CELL) + 4*CELL;
    dn_set_cfa(dn_lookup_word("HERE"), cfa);
    dn_set_cfa(dn_lookup_word("SP@"), cfa);
    dn_set_cfa(dn_lookup_word("RP@"), cfa);

    dn_create_variable("PAD", 0);
    dn_allot(96);
}
