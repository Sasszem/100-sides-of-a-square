#include "../forth.h"
#include "../simple_compile.h"


static void word_tick_int() {
    // ptr and count is on stack
    dn_cell_t count = dn_pop();
    dn_cell_t ptr = dn_pop();

    dn_push(dn_lookup_word_counted((const char*)&DN_FORTH_MEMORY[ptr], count));
}

static void word_disas() {
    dn_disas_word(dn_lookup_word_containing_address(dn_pop()));
}

void words_set_words() {
    DN_FORTH_WORD("COMPILE,", ",");
    
    dn_create_system_word("[[']]", word_tick_int);
    dn_word_set_hidden();

    DN_FORTH_WORD("'", "PARSE-NAME", "[[']]");
    
    dn_create_system_word("[DISAS]", word_disas);
    dn_word_set_hidden();

    {
        dn_cell_t cfa = (
            DN_FORTH_WORD("DISAS",
                "'", "DUP", "0=", "[?JUMP]", (T_IMMEDIATE)0,
                    0, "COUNT", "TYPE", "EXIT",
                "[DISAS]"
            )
        );
        dn_allot_counted("Word not found!\n");
        dn_write_cell(cfa+4*CELL, cfa+10*CELL);
        dn_write_cell(cfa+6*CELL, cfa+12*CELL);
    }


    DN_FORTH_WORD("(", ')', "PARSE", "2DROP");
    dn_word_set_immediate();

    DN_FORTH_WORD(".(", ')', "PARSE", "TYPE");
    dn_word_set_immediate();

    {
        dn_cell_t cfa = (
            DN_FORTH_WORD("FIND", 
                // c-addr
                "DUP", "COUNT", "[[']]",
                // c-addr xt?
                "DUP", "0=", "[?JUMP]", (T_IMMEDIATE)0,
                    "EXIT",
                // c-addr xt
                "SWAP", "DROP", "DUP",
                // xt xt
                // fetch it
                "@", _WORD_IS_IMMEDIATE, "AND",
                "0=", "[?JUMP]", (T_IMMEDIATE)0,
                    // xt, not imm
                    -1, "EXIT",
                1
            )
        );
        dn_write_cell(cfa + 6*CELL, cfa+8*CELL);
        dn_write_cell(cfa + 17*CELL, cfa+21*CELL);
    }

    DN_FORTH_WORD("[CHAR]",
        "PARSE-NAME", "DROP", "C@", dn_lookup_word("[LITERAL]"), "COMPILE,", "COMPILE,"
    );
    dn_word_set_immediate();

    DN_FORTH_WORD("LITERAL",
        dn_lookup_word("[LITERAL]"), "COMPILE,", ","    
    );
    dn_word_set_immediate();

    {
        dn_cell_t cfa = (
            DN_FORTH_WORD("C\"",
                '"', "PARSE",
                // (addr cnt)
                // add LITERAL, val, JUMP, TARGET (and remember target pos)
                dn_lookup_word("[LITERAL]"), "COMPILE,",
                "HERE", 3, "CELLS", "+", ",",
                dn_lookup_word("[JUMP]"), "COMPILE,",
                "HERE", 1, "CELLS", "ALLOT", // addr cnt JUMP
                // store CNT
                "ROT", "ROT", "DUP", "C,", // jump addr cnt
                0, "[DOSET]", "[DO]", (T_IMMEDIATE)0,
                    // jump addr
                    "DUP", "I", "+", "C@", "C,",
                "[LOOP]", (T_IMMEDIATE)0, "UNLOOP", "DROP", // jump
                "ALIGN", "HERE", "SWAP", "!"
            )
        );
        dn_write_cell(cfa+28*CELL, cfa+36*CELL);
        dn_write_cell(cfa+35*CELL, cfa+27*CELL);
        dn_word_set_immediate();
    }

    DN_FORTH_WORD("S\"", 
        "C\"", dn_lookup_word("COUNT"), ","
    );
    dn_word_set_immediate();

    dn_create_variable("STATE", 0);
    // TRUE IF COMPILE

    DN_FORTH_WORD("[", "FALSE", "STATE", "!");
    dn_word_set_immediate();

    DN_FORTH_WORD("]", "TRUE", "STATE", "!");

        {
        dn_cell_t cfa = (
            DN_FORTH_WORD("INTERPRET", 
                "WORD", // return counted str
                "DUP", "COUNT", "SWAP", "DROP",
                // (cstr, count)
                "0=", "[?JUMP]", (T_IMMEDIATE)0,
                    // if count==0, could not read...
                    "SOURCE-ID", "[?JUMP]", (T_IMMEDIATE)0,
                        "DROP", "exit",
                    "BYE",
                // cstr
                "FIND", 
                // xt found?
                "DUP", "[?JUMP]", (T_IMMEDIATE)0, 
                    // if found
                    // compile only if state is -1 and result is -1
                    // valid values for state: -1 (compiling), 0 (executing)
                    // valid values for result: -1 (normal) 1 (imm)
                    // comp = normal -> true; comp = imm -> false
                    // exec = normal -> false; exec = imm -> false
                    "STATE", "@", "=", "[?JUMP]", (T_IMMEDIATE)0,
                        "COMPILE,",
                    "[JUMP]", (T_IMMEDIATE)0,
                        "EXECUTE",
                        "[JUMP]", (T_IMMEDIATE)0, 
                    // else - xt not found
                    "DROP", "COUNT", "NUMBER?", "[?JUMP]", (T_IMMEDIATE)0,
                        // valid number
                        "STATE", "@", "[?JUMP]", (T_IMMEDIATE)0,
                            "LITERAL",
                            "[JUMP]", (T_IMMEDIATE)0,
                        'x', "EMIT", "CR", "DROP",
                // loop to begin
                "[JUMP]", (T_IMMEDIATE)0))
        ;
        dn_write_cell(cfa+7*CELL, cfa+14*CELL);
        dn_write_cell(cfa+10*CELL, cfa+13*CELL);
        dn_write_cell(cfa + 17*CELL, cfa+29*CELL);

        dn_write_cell(cfa+22*CELL, cfa+26*CELL);
        dn_write_cell(cfa+25*CELL, cfa+27*CELL);

        dn_write_cell(cfa + 28*CELL, cfa+46*CELL);
        dn_write_cell(cfa + 33*CELL, cfa+41*CELL);
        dn_write_cell(cfa+37*CELL, cfa+46*CELL);
        dn_write_cell(cfa+40*CELL, cfa+46*CELL);
        dn_write_cell(cfa+47*CELL, cfa);

        dn_set_var("[INTERP]", dn_lookup_word("INTERPRET"));
    }

    {
        dn_cell_t cfa = (
            DN_FORTH_WORD(".\"",
                "STATE", "@", "[?JUMP]",(T_IMMEDIATE)0,
                    // if compile state
                    "S\"", dn_lookup_word("TYPE"), "COMPILE,", "EXIT",
                // if interpret state
                '"', "PARSE", "TYPE"
            )
        );
        dn_word_set_immediate();
        dn_write_cell(cfa + 3*CELL, cfa+9*CELL);
    }

    DN_FORTH_WORD("[']",
        "'", "LITERAL" 
    );
    dn_word_set_immediate();

    {
        dn_cell_t cfa = (
            DN_FORTH_WORD("POSTPONE",
                "WORD", "FIND",
                // now we have (maybe-xt flag)
                // if not found, do nothing
                "DUP", "[?JUMP]", (T_IMMEDIATE)0,
                    // found it, compile
                    // test if immediate
                    -1, "=", "[?JUMP]", (T_IMMEDIATE)0,
                        // immediate found
                        "COMPILE,", "EXIT",
                    // not immed
                    // compile code that will compile code...
                    // (xt)
                    "LITERAL", dn_lookup_word("COMPILE,"), "COMPILE,"
                // exit
            )
        );
        dn_write_cell(cfa+4*CELL, cfa+16*CELL);
        dn_write_cell(cfa+9*CELL, cfa+12*CELL);
        dn_word_set_immediate();
    }

    DN_FORTH_WORD("[HIDE]", 
        "DP", "@", // xt on stack
        "DUP", "@", 
        _WORD_IS_SKIP, "OR",
        "SWAP", "!"
    );
    dn_word_set_hidden();


    DN_FORTH_WORD("[REVEAL]", 
        "DP", "@", // xt on stack
        "DUP", "@", 
        ~_WORD_IS_SKIP, "AND",
        "SWAP", "!"
    );
    dn_word_set_hidden();

    {
        dn_cell_t cfa = (
            DN_FORTH_WORD("CREATE",
                // get a name
                "PARSE-NAME", // (c-addr cnt)
                // limit CNT
                127, "AND",
                "DUP", 
                128, "OR", 24, "LSHIFT",
                "DP", "@", "OR", 
                "ALIGN",
                ",", 0, "DUP", ",", ",",
                "HERE", 3, "CELLS", "-", "DP", "!",

                // header is set...
                // set up name
                // (c-addr cnt)
                0, "[DOSET]", "[DO]", (T_IMMEDIATE)0,
                    // c-addr
                    "DUP", "I", "+", "C@", "C,",
                "[LOOP]", (T_IMMEDIATE)0, "UNLOOP",
                "DROP", "ALIGN",

                // set up PFA
                "HERE", "DP", "@", "CELL+", "!"
            )
        );
        dn_write_cell(cfa+32*CELL, cfa+40*CELL);
        dn_write_cell(cfa+39*CELL, cfa+31*CELL);
    }

    DN_FORTH_WORD("VARIABLE", 
        "CREATE", 1, "CELLS", "ALLOT"
    );

    DN_FORTH_WORD("DOES>",
        // set last entry's CFA to own return position & exit
        "R>",
        "DP", "@", 2, "CELLS", "+", // cfa-ptr
        "!"
    );

    DN_FORTH_WORD("CONSTANT",
        "CREATE", ",", "DOES>", "@"
    );

    DN_FORTH_WORD("BUFFER:", "CREATE", "ALLOT");

    DN_FORTH_WORD("MARKER",
        "DP", "@", "CREATE", ",", "DOES>", "@", "DP", "!"
    );

    DN_FORTH_WORD(":",
        "CREATE", 
        "HERE", "DP", "@", "CELL+", "CELL+", "!",
        dn_lookup_word("DROP"), "COMPILE,", 
        "[HIDE]",
        "]"
    );

    DN_FORTH_WORD(";", 
        dn_lookup_word("EXIT"), "COMPILE,",
        "[REVEAL]",
        "["
    );
    dn_word_set_immediate();

/*
    // problem: restore-input does not know how many stack entries have been added
    DN_FORTH_WORD("EVALUATE",
        // c-addr u
        "SAVE-INPUT",
        6, "ROLL",
        6, "ROLL",
        -1, "[SRC-ID]", "!",
        "[SRC-CNT]", "!",
        "[SRC]", "!",
        0, ">IN", "!",
        "INTERPRET",
        "RESTORE-INPUT"
    );
*/
    {
        dn_cell_t cfa= (
            DN_FORTH_WORD("WORDS",
                // get DP
                "DP", "@",
                // LOOP begin
                    // current XT
                    "DUP", "0=", "[?JUMP]", (T_IMMEDIATE)0,
                        // if XT is empty
                        "DROP", "CR", "EXIT",
                    // xt not 0
                    "DUP", "@", // xt header
                    "DUP", (_WORD_IS_HIDDEN | _WORD_IS_SKIP), "AND", "0=", "[?JUMP]", (T_IMMEDIATE)0,
                        // not hidden or skip
                        "OVER", 3, "CELLS", "+", // xt header NAME
                        "OVER", 24, "RSHIFT", 127, "AND", // xt header NAME cnt
                        "TYPE", "SPACE",
                    // anyway, printed or not
                    // update XT
                    // (xt header)
                    "NIP",
                    ((1<<18)-1), "AND",
                "[JUMP]", (T_IMMEDIATE)0
            )
        );
        dn_write_cell(cfa+5*CELL, cfa+9*CELL);
        dn_write_cell(cfa+17*CELL, cfa+32*CELL);
        dn_write_cell(cfa+37*CELL, cfa+2*CELL);
    }
}