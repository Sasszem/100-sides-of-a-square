#include "forth_words.h"
#include "../simple_compile.h"

void words_set_control() {
    DN_FORTH_WORD("I", "2R@", "DROP");
    DN_FORTH_WORD("J", 
        // () (R: lim1 i1 lim2 i2 retp)
        "2R>", "2R@", // (i2 retp i1 lim2) (lim1 i1 lim2)
        "DROP", // (i2 retp i1) (lim1 i1 lim2)
        "ROT", "ROT", // (i1 i2 retp) (lim1 i1 lim2)
        "2>R" // (i1) (lim1 i1 lim2 i2 retp)
    );
    
    DN_FORTH_WORD("[DOSET]", "R>", "ROT", ">R", "2>R");
    dn_word_set_hidden();

    DN_FORTH_WORD("[IMMED]", "2R>", "OVER", "CELL+", ">R", ">R", "@");
    dn_word_set_hidden();
    
    DN_FORTH_WORD("[LITERAL]", "[IMMED]");
    dn_word_set_hidden();
    
    {
    dn_cell_t cfa = (
        DN_FORTH_WORD("[DO]", "[IMMED]", 
            "R>", // (jump ret) (lim i)
            "2R>", // jump ret lim i
            "2DUP", // jump ret lim i lim i
            "2>R", // jump ret lim i (lim i)
            "-", // jump ret (lim-i) (lim i)
            "[?JUMP]", (T_IMMEDIATE)0, // jump ret
            // if nonzero, i.e. i!=limit, so don't jump
            // (jump ret)
            "SWAP",
            // THEN
            "DROP", ">R"
        ));
        dn_write_cell(cfa + 7*CELL, cfa+9*CELL);
        dn_word_set_hidden();
    }

    {
        dn_cell_t cfa = (DN_FORTH_WORD("[LOOP]", 
            // () (limit idx ret)
            "[IMMED]", // (jump) (limit idx, ret')
            "2R>", // (jump idx ret') (limit)
            "SWAP", // (jump ret idx) (limit)
            "1+", // (jump ret idx') (limit)
            ">R", // (jump ret) (limit idx')
            "DROP", // (jump) (limit idx')
            ">R" // (limit idx' jump)
        ));
        dn_word_set_hidden();
    }

    
    DN_FORTH_WORD("UNLOOP", "R>", "2R>", "2DROP", ">R");
    DN_FORTH_WORD("IMMEDIATE", "DP", "@", "DUP", "@", _WORD_IS_IMMEDIATE, "OR", "SWAP", "!");


}
void words_set_control_2() {

    DN_FORTH_WORD(">>>R",
        // (x -- )(r2 r1 -- x r2 r1)
        "2R>", "ROT", ">R", "2>R"
    );
    dn_word_set_hidden();

    DN_FORTH_WORD("R>>>",
        // (--x) (x r2 r1 -- r2 r1)
        "2R>", "R>", "ROT", "ROT", "2>R"
    );

    DN_FORTH_WORD("IF",
        dn_lookup_word("[?JUMP]"), "COMPILE,",
        "HERE", ">>>R", 0, ","
    );
    dn_word_set_immediate();

    DN_FORTH_WORD("THEN",
        "HERE", "R>>>", "!"
    );
    dn_word_set_immediate();

    DN_FORTH_WORD("ELSE",
        dn_lookup_word("[JUMP]"), "COMPILE,",
        "HERE",
        0, ",",
        "HERE", "R>>>", "!",
        ">>>R"
    );
    dn_word_set_immediate();

    DN_FORTH_WORD("RECURSE",
        "DP", "@", "COMPILE,"
    );
    dn_word_set_immediate();

    DN_FORTH_WORD("BEGIN",
        "HERE", ">>>R"
    );
    dn_word_set_immediate();

    DN_FORTH_WORD("UNTIL",
        dn_lookup_word("[?JUMP]"), "COMPILE,",
        "R>>>", ","
    );
    dn_word_set_immediate();

    DN_FORTH_WORD("AGAIN",
        dn_lookup_word("[JUMP]"), "COMPILE,",
        "R>>>", ","
    );
    dn_word_set_immediate();

    DN_FORTH_WORD("WHILE",
        dn_lookup_word("[?JUMP]"), "COMPILE,",
        "R>>>",
        "HERE", ">>>R", ">>>R",
        0, ","
    );
    dn_word_set_immediate();

    DN_FORTH_WORD("REPEAT",
        dn_lookup_word("[JUMP]"), "COMPILE,",
        "R>>>", ",",
        "HERE", "R>>>", "!"
    );
    dn_word_set_immediate();

    DN_FORTH_WORD("?DO",
        dn_lookup_word("[DOSET]"), "COMPILE,",
        dn_lookup_word("[DO]"), "COMPILE,",
        "HERE", ">>>R", 0, ","
    );
    dn_word_set_immediate();

    DN_FORTH_WORD("DO",
        dn_lookup_word("[DOSET]"), "COMPILE,",
        dn_lookup_word("[JUMP]"), "COMPILE,",
        "HERE", 3, "CELLS", "+", ",",
        dn_lookup_word("[DO]"), "COMPILE,",
        "HERE", ">>>R", 0, ","
    );
    dn_word_set_immediate();


    DN_FORTH_WORD("LOOP",
        dn_lookup_word("[LOOP]"), "COMPILE,",
        "R>>>", "DUP", -1, "CELLS", "+", ",",
        "HERE", "SWAP", "!",
        dn_lookup_word("UNLOOP"), "COMPILE,"
    );
    dn_word_set_immediate();
}