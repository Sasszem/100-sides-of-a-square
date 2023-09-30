#include "forth.h"
#include <stdio.h>


#ifndef PRINT_EXECUTE_DEBUG
#define PRINT_EXECUTE_DEBUG 0
#endif

void dn_execute_token(dn_cell_t xt) {
    dn_set_var("[XT]", xt);
    while(1) {
        // loop: XT is set up
        #if PRINT_EXECUTE_DEBUG == 1
                printf("\n[IP]=%05x (%05x)\n", dn_get_var("[IP]") - CELL, dn_read_cell(dn_get_var("[IP]")-CELL));
                printf("XT is at %x: '%.*s'\n", xt, dn_word_get_name_len(xt), dn_word_get_name_ptr(xt));
                printf("\t"); dn_debug_dump_stack();
                printf("\t"); dn_debug_dump_ret_stack();
        #endif
        dn_cell_t header = dn_read_cell(xt);
        if (!dn_is_word(xt)) {
            dn_abort("INVALID PTR");
            // printf("Error: ptr %x is not a word!\n", xt);
            return;
        }
        if (header & _WORD_IS_PRIMITIVE) {
            #if PRINT_EXECUTE_DEBUG == 1
            printf("\e[41m");
            #endif
            
            dn_word_get_fptr_from_xt(xt)();

            #if PRINT_EXECUTE_DEBUG == 1
            printf("\e[49m");
            #endif

        } else {
            // push CFA
            dn_cell_t cfa = dn_read_cell(xt + 2*CELL);
            dn_cell_t pfa = dn_read_cell(xt + 1*CELL);
            dn_push(pfa);
            if (cfa) {
                // now start executing words
                // set XT to 1st word
                // push next XT address
                dn_rpush(dn_get_var("[IP]"));
                dn_set_var("[IP]", cfa);
            }
        }


        // executed a word
        // now if we have a return ptr
        // start executing next word pointed by ptr
        // and increment PTR by 1 cell
        if (!dn_get_var("[IP]"))
        {
            break;
        } else {
            dn_cell_t addr = dn_get_var("[IP]"); 
            xt = dn_read_cell(addr);
            dn_set_var("[XT]", xt);
            dn_set_var("[IP]", addr + CELL);
        }
    }
    dn_rpop();
}


void dn_execute(const char* name) {
    dn_cell_t xt = dn_lookup_word(name);
    if (!xt) {
        printf("Could not execute word '%s'\n", name);
    }
    dn_execute_token(xt);
} 

void dn_abort(const char* msg) {
    if (!dn_get_var("[ABORT?]")) {
        dn_set_var("[ABORT?]", -1);

        if (msg)
            printf("\n--- %s ---\n", msg);
        
        dn_print_backtrace();
        dn_set_var("[IP]", dn_read_cell(dn_lookup_word("ABORT")+2*CELL)+CELL);
    }
}