#include<stdio.h>
#include <string.h>
#include "forth.h"

#include "md5.h"
#include "simple_compile.h"


void test_pyr() {

    dn_cell_t cfa = (DN_FORTH_WORD("PYR", 
        0, 
        "[DOSET]", "[DO]", (T_IMMEDIATE)0,  // created by DO, immediate is end-of-loop address
            "I", "1+", 0,
            "[DOSET]", "[DO]", (T_IMMEDIATE)0,  // created by DO, immediate is end-of-loop address
                "STAR",
            "[LOOP]", (T_IMMEDIATE)0, "UNLOOP", // created by LOOP
        "CR",
        "[LOOP]", (T_IMMEDIATE)0, "UNLOOP"
        ));
    dn_cell_t outer_loop_begin = cfa + 3*CELL;
    dn_cell_t outer_loop_end = cfa + 19*CELL;

    dn_cell_t inner_loop_begin = cfa + 10*CELL;
    dn_cell_t inner_loop_end = cfa + 15*CELL;

    dn_write_cell(outer_loop_begin+CELL, outer_loop_end);
    dn_write_cell(outer_loop_end-CELL, outer_loop_begin);

    dn_write_cell(inner_loop_begin+CELL, inner_loop_end);
    dn_write_cell(inner_loop_end-CELL, inner_loop_begin);

    dn_disas_word(dn_lookup_word("PYR"));

    dn_push(10);
    dn_execute("PYR");
}





void test_jumps() {
    DN_FORTH_WORD("PTRUE", (int)'T', "EMIT", (int)'R', "EMIT", (int)'U', "EMIT", (int)'E', "EMIT");
    DN_FORTH_WORD("PFALSE", (int)'F', "EMIT", (int)'A', "EMIT", (int)'L', "EMIT", (int)'S', "EMIT", (int)'E', "EMIT");



    dn_cell_t test_cfa = (DN_FORTH_WORD("TEST", "[?JUMP]", (T_IMMEDIATE)0, "PTRUE", "[JUMP]", (T_IMMEDIATE)0, "PFALSE", "CR"));
    dn_write_cell(test_cfa + CELL, test_cfa + 5*CELL);
    dn_write_cell(test_cfa + 4*CELL, test_cfa + 6*CELL);

    dn_disas_word(dn_lookup_word("PTRUE"));
    dn_disas_word(dn_lookup_word("PFALSE"));
    dn_disas_word(dn_lookup_word("TEST")),

    dn_push(5);
    dn_execute("TEST"); // prints "TRUE"

    dn_push(0);
    dn_execute("TEST"); // prints "FALSE"
}

void test_disas() {
    dn_disas_word(dn_lookup_word("EMIT"));
    dn_disas_word(dn_lookup_word("TEST"));
    dn_disas_word(dn_lookup_word("PAD"));
    dn_disas_word(dn_lookup_word("PTRUE"));
}


void test_ret_stack_words() {
    dn_create_system_word(".RR", dn_debug_dump_ret_stack);
    DN_FORTH_WORD("ret-test", ".RR", ".S", "CR", "2>R", ".RR", ".S", "CR", "2R@", "2R>", ".RR", ".S", "CR");
    dn_push(1);
    dn_push(2);
    dn_push(3);
    dn_execute("ret-test");
}

void test_simple_for() {
    dn_cell_t cfa = (
        DN_FORTH_WORD("DO-TEST", 10,0, "[DOSET]", "[DO]", (T_IMMEDIATE)0,"I", ".", "[LOOP]", (T_IMMEDIATE)0, "UNLOOP", "CR")
    );
    dn_write_cell(cfa + 6*CELL, cfa+11*CELL);
    dn_write_cell(cfa + 10*CELL, cfa+5*CELL);
    dn_disas_word(dn_lookup_word("DO-TEST"));
    dn_execute("DO-TEST");
}

void test_format_out() {
    DN_FORTH_WORD("FORMAT-TEST", "<#", "#", "#", ':', "HOLD", "#S",'@', "HOLD", "[#PTR]", "@", "[#CNT]", "@", "HOLDS", "#>", 3, "SPACES", "TYPE", "CR");
    dn_push(123456);
    dn_execute("S>D");
    dn_execute("2DUP");
    dn_execute("DECIMAL");
    dn_execute("FORMAT-TEST");
    dn_execute("HEX");
    dn_execute("FORMAT-TEST");

    dn_execute("DECIMAL");
    dn_push(-10);
    dn_push(-20);
    dn_push(-30);
    dn_execute(".S");
    dn_execute("CR");
}

void test_aux_out() {
    
    dn_push(-1);
    dn_push(5);
    dn_execute(".R");
    dn_execute("CR");
    
    printf("Now the second...\n");

    dn_push(123);
    dn_push(5);
    dn_execute(".R");
    dn_execute("CR");

    printf("And the 3rd\n");
    dn_push(123456);
    dn_push(5);
    dn_execute(".R");
    dn_execute("CR");

}

void test_nested_loops() {
        {
        dn_cell_t cfa = (DN_FORTH_WORD("DO-TEST2",
            11, 1, "[DOSET]", "[DO]", (T_IMMEDIATE)0,
                "I", 2, ".R", ':', "EMIT", "SPACE",
                11, 1, "[DOSET]", "[DO]", (T_IMMEDIATE)0,
                    "I", "J", "*", 4 ,".R",
                "[LOOP]", (T_IMMEDIATE)0, "UNLOOP",
                "CR",
            "[LOOP]", (T_IMMEDIATE)0, "UNLOOP"
        ));
        dn_write_cell(cfa+6*CELL, cfa+34*CELL);
        dn_write_cell(cfa+33*CELL, cfa+5*CELL);
 
        dn_write_cell(cfa+21*CELL, cfa+30*CELL);
        dn_write_cell(cfa+29*CELL, cfa+20*CELL);
 
        dn_execute("IMMEDIATE");
        dn_disas_word(dn_lookup_word("DO-TEST2"));
        dn_execute("DO-TEST2");
    }
}

void test_fill_erase_move() {
    dn_write_cell(0xf000, 0x12345678);
    dn_write_cell(0xf004, 0xcafebabe);

    dn_push(0xf000);
    dn_push(0xf00a);
    dn_push(8);
    dn_execute("MOVE");

    dn_push(0xf000);
    dn_push(4);
    dn_push('A');
    dn_execute("FILL");

    dn_push(0xf004);
    dn_push(4);
    dn_execute("ERASE");

    dn_dump_cells(0xf000, 10);
}

void test_execute() {
    dn_disas_word(dn_lookup_word("EXECUTE"));
    dn_push(5);
    dn_push(2);
    dn_push(dn_lookup_word("*"));
    dn_execute("EXECUTE");
    dn_disas_word(dn_lookup_word("EXECUTE"));
    dn_push(dn_lookup_word("."));
    dn_execute("EXECUTE");
    dn_execute("CR");
    dn_create_system_word(".RR", dn_debug_dump_ret_stack);

    DN_FORTH_WORD(">>R", 
        // (r1 r2) -- (x r1 r2)
        "2R>", "ROT", ">R", "2>R"
    );
    DN_FORTH_WORD("R>>", 
        // (x r1 r2)
        "R>", // (r2) (x r1)
        "2R>", // (r2 x r1)
        "ROT", // (x r1 r2)
        "2>R"
    );
    DN_FORTH_WORD("EX-TEST", 1, dn_lookup_word(">R"), "EXECUTE", ".RR", dn_lookup_word("R>"), "EXECUTE", ".");
    dn_execute("EX-TEST");
    dn_disas_word(dn_lookup_word("EX-TEST"));
}

void test_parsing() {
        dn_create_variable("MY-STR", 0);
    dn_allot(-1*CELL);
    dn_allot_str("A BC DEF     ");

    dn_create_variable("MY-STR-2", 0);
    dn_allot(-1*CELL);
    dn_allot_str("AAAAAAAAAAAAAAAAA");
    
    dn_set_var("[SRC]", dn_read_cell(dn_lookup_word("MY-STR")+CELL));
    dn_set_var("[SRC-CNT]", 8);
    dn_set_var("[SRC-ID]", -1);
    
    // DN_FORTH_WORD("TT", "KEY","EMIT", "KEY", "EMIT", "KEY", "EMIT", "CR", "SOURCE", "HEX", ".S", "CR");
    /*
    DN_FORTH_WORD("ACCEPT-TEST", "MY-STR-2", 6, "ACCEPT", "MY-STR-2", "SWAP", "2DUP", "TYPE", ".S", "CR");
    dn_execute("ACCEPT-TEST");
    dn_dump_cells(dn_lookup_word("[TIB-CNT]"), 20);
    */
    dn_execute("PARSE-NAME");
    dn_push(':');
    dn_execute("EMIT");
    dn_execute("TYPE");
    dn_push(':');
    dn_execute("EMIT");

    dn_execute("PARSE-NAME");
    dn_push(':');
    dn_execute("EMIT");
    dn_execute("TYPE");
    dn_push(':');
    dn_execute("EMIT");

    dn_execute("PARSE-NAME");
    dn_push(':');
    dn_execute("EMIT");
    dn_execute("TYPE");
    dn_push(':');
    dn_execute("EMIT");
}

extern void setup_chall();

int main() {
    setbuf(stdout, NULL);
    
    dn_init();
    setup_chall();
    dn_rpush(0);
    dn_execute("?");
    dn_execute("ABORT");
    
    fflush(stdout);
    return 0;
}
