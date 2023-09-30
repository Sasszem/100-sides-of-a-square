#include "forth_words.h"
#include "../simple_compile.h"
#include <stdio.h>
#include <stdlib.h>

static void word_exit() {
    dn_cell_t retp = 0;
    if (dn_get_var("RP@") < dn_get_var("R0"))
        retp = dn_rpop();
    dn_set_var("[IP]", retp);
}

static void word_literal_int() {
    dn_push(_get_immediate_val());
}

static void word_condjump() {
    dn_cell_t target = _get_immediate_val();

    if (dn_pop()==0 && !dn_get_var("[ABORT?]")) {
        dn_set_var("[IP]", target);
    }
}

static void word_jump() {
    dn_cell_t target = _get_immediate_val();
    dn_set_var("[IP]", target);
}

static void word_execute() {
    dn_execute_token(dn_pop());
}

static void word_bye() {
    fflush(stdout);
    exit(0);
}


void words_set_core() {
    dn_create_system_word("[?JUMP]", word_condjump);
    dn_word_set_hidden();

    dn_create_system_word("[JUMP]", word_jump);
    dn_word_set_hidden();

    dn_create_system_word("EXIT", word_exit);
    dn_create_system_word("EXECUTE", word_execute);


    dn_create_system_word("[LITERAL]", word_literal_int);
    dn_word_set_hidden();

    dn_create_system_word("BYE", word_bye);
}

void words_set_core_aux() {
    DN_FORTH_WORD("C,", "HERE", "C!", 1, "CHARS", "ALLOT");
    DN_FORTH_WORD(",", "HERE", "!", 1, "CELLS", "ALLOT");

    DN_FORTH_WORD(">BODY", "CELL+", "@");

    dn_create_variable("[INTERP]", 0);
    dn_word_set_hidden();

    DN_FORTH_WORD("QUIT", dn_read_cell(dn_lookup_word("R0")+CELL), "@", dn_read_cell(dn_lookup_word("RP@")+CELL), "!", 0, "[ABORT?]", "!", "[INTERP]", "@", "EXECUTE");
    DN_FORTH_WORD("ABORT", dn_read_cell(dn_lookup_word("S0")+CELL), "@", dn_read_cell(dn_lookup_word("SP@")+CELL), "!","QUIT");

    dn_create_system_word("BT", dn_print_backtrace);
}