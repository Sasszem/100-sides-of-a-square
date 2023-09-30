#include "forth_words.h"
#include "../simple_compile.h"
#include <stdint.h>
#include <stdio.h>

static void word_emit() {
    uint8_t chr = dn_pop()&0xff;
    if (chr!=0xff)
        printf("%c", chr);
}

static void word_refill() {
    if (dn_get_var("[SRC-ID]") == 0) {
        dn_cell_t pfa_ptr = dn_read_cell(dn_lookup_word("[TIB]") + CELL);
        int no_chars_read;
        for (no_chars_read = 0; no_chars_read < 99; no_chars_read++) {
            int char_read = getchar();
            if (char_read == EOF) {
                if (no_chars_read == 0) {
                    dn_push(0);
                    return;
                }
                break;
            }
            dn_write_char(pfa_ptr + no_chars_read, char_read);
            if (char_read == '\n') {
                no_chars_read++;
                break;
            }
        }
        dn_set_var("[TIB-CNT]", no_chars_read);
        dn_set_var(">IN", 0);
        dn_push(-1);
        return;
    }
    dn_push(0);
}

void words_set_io() {
    dn_create_system_word("EMIT", word_emit);
}

void words_set_derived_io() {
    dn_create_variable("[#CNT]", 0);
    dn_word_set_hidden();

    dn_create_variable("[#PTR]", 0);
    dn_word_set_hidden();

    dn_create_variable("BASE", 10);

    DN_FORTH_WORD("HEX", 16, "BASE", "!");
    DN_FORTH_WORD("DECIMAL", 10, "BASE", "!");

    DN_FORTH_WORD("BL", ' ');
    DN_FORTH_WORD("SPACE", "BL", "EMIT");
    DN_FORTH_WORD("CR", (int)'\n', "EMIT");

    {
        dn_cell_t cfa = (
            DN_FORTH_WORD("SPACES", 
                // note: this is not standard compliant, as length could be unsigned...
                0, "MAX", 0,
                "[DOSET]", "[DO]", (T_IMMEDIATE)0,
                    "SPACE",
                "[LOOP]", (T_IMMEDIATE)0, "UNLOOP"
            )
        );
        dn_write_cell(cfa + 7*CELL, cfa+11*CELL);
        dn_write_cell(cfa + 10*CELL, cfa+6*CELL);
    }

    {
        dn_cell_t cfa = (DN_FORTH_WORD("[#EXT]", "DUP", 10, "<", "[?JUMP]", (T_IMMEDIATE)0, 48, "+", "EXIT", 10, "-", 'A', "+"));
        dn_write_cell(cfa + 5*CELL, cfa+10*CELL);
        dn_word_set_hidden();
    }

    DN_FORTH_WORD("<#",
        0, "[#CNT]", "!",
        "HERE", "[#PTR]", "!"
    );

    DN_FORTH_WORD("HOLD", 
        "[#CNT]", "DUP", "@", "DUP", "CHAR+", "ROT", "!",
        "[#PTR]", "@", "+", "C!"
    );

    {
        // (addr cnt -- )
        dn_cell_t cfa = (
            DN_FORTH_WORD("HOLDS", 
                0, // addr cnt 0
                "[DOSET]", "[DO]", (T_IMMEDIATE)0,
                    // addr
                    "DUP", "C@", "HOLD", "1+",
                "[LOOP]", (T_IMMEDIATE)0, "UNLOOP",
                "DROP"
            )
        );
        dn_write_cell(cfa + 4*CELL, cfa+11*CELL);
        dn_write_cell(cfa + 10*CELL, cfa+3*CELL);
    }


    DN_FORTH_WORD("#",
        // convert using BASE
        "BASE", "@", "UM/MOD2", "ROT", // now have last digit on top
        // convert remainder to "external form", and add to output buffer
        "[#EXT]",
        "HOLD"
    );

    {
        dn_cell_t cfa = (DN_FORTH_WORD("#S", "#", "2DUP", "0=", "SWAP", "0=", "AND", "[?JUMP]", (T_IMMEDIATE)0));
        dn_write_cell(cfa + 7*CELL, cfa);
    }

    {
        dn_cell_t cfa = (DN_FORTH_WORD("#>", 
            "2DROP",
            "[#PTR]", "@", "DUP", "[#CNT]", "@", 2, "*", "+", "1-", "SWAP", // (ptr+2*count ptr)
            "[#CNT]", "@", 0,
            "[DOSET]", "[DO]", (T_IMMEDIATE)0,
                // (ptr+2*cnt ptr)
                "DUP", "I", "+", "C@",
                2, "PICK", "I", "-", "C!",
            "[LOOP]", (T_IMMEDIATE)0, "UNLOOP",
            "SWAP", "DROP", "[#CNT]", "@", "SWAP", "OVER", "+", "SWAP" // ,, "[#CNT]", "@", "OVER", "+", "SWAP"
            )
        );
        dn_write_cell(cfa+18*CELL, cfa+31*CELL);
        dn_write_cell(cfa+30*CELL, cfa+17*CELL);
    }

    {
        dn_cell_t cfa = (
            DN_FORTH_WORD("TYPE", 
                // note: this is not standard compliant, as length could be unsigned...
                0, "MAX", 0,
                "[DOSET]", "[DO]", (T_IMMEDIATE)0,
                    // (base-addr)s
                    "DUP", "C@", "EMIT", "1+",
                "[LOOP]", (T_IMMEDIATE)0, "UNLOOP",
                "DROP"
            )
        );
        dn_write_cell(cfa + 7*CELL, cfa+14*CELL);
        dn_write_cell(cfa + 13*CELL, cfa+6*CELL);
    }

    {
        dn_cell_t cfa = (
            DN_FORTH_WORD("SIGN",
                (1<<31), "AND", "[?JUMP]", (T_IMMEDIATE)0, '-', "HOLD"
            )
        );
        dn_write_cell(cfa+4*CELL, cfa+8*CELL);
    }

    DN_FORTH_WORD("[.]", "DUP", "ABS", 0, "<#", "#S", "ROT", "SIGN", "#>");
    dn_word_set_hidden();

    DN_FORTH_WORD(".", "[.]", "TYPE", "SPACE");

    {
        dn_cell_t cfa = (
            DN_FORTH_WORD(".S", "DEPTH", 
            
                "DUP", 0, "<#", '>', "HOLD", "#S", '<', "HOLD", "#>", "TYPE", "SPACE", // print <depth>
                "DUP", 0,  // (depth depth 0)
                "[DOSET]", "[DO]", (T_IMMEDIATE)0,
                    // (depth)
                    "DUP", "I", "-", "PICK", ".",
                "[LOOP]", (T_IMMEDIATE)0, "UNLOOP",
                "DROP"
        ));
        dn_write_cell(cfa+20*CELL, cfa+28*CELL);
        dn_write_cell(cfa+27*CELL, cfa+19*CELL);
    }

    DN_FORTH_WORD("U.", 0,"<#", "#S", "#>", "TYPE", "SPACE");

    {
        dn_cell_t cfa = (
            DN_FORTH_WORD(".R", "SWAP", "[.]", "2DROP",
            "[#CNT]", "@", "-", 0, "MAX", 0,
            "[DOSET]", "[DO]", (T_IMMEDIATE)0,
                ' ', "HOLD",
            "[LOOP]", (T_IMMEDIATE)0, "UNLOOP",
            "#>", "TYPE"
            )
        );
        dn_write_cell(cfa+13*CELL, cfa+19*CELL);
        dn_write_cell(cfa+18*CELL, cfa+12*CELL);
    }

    {
        dn_cell_t cfa = (
            DN_FORTH_WORD("U.R", "SWAP", 0, "<#", "#S", "#>", "2DROP",
            "[#CNT]", "@", "-", 0, "MAX", 0,
            "[DOSET]", "[DO]", (T_IMMEDIATE)0,
                ' ', "HOLD",
            "[LOOP]", (T_IMMEDIATE)0, "UNLOOP",
            "#>", "TYPE"
            )
        );
        dn_write_cell(cfa+17*CELL, cfa+23*CELL);
        dn_write_cell(cfa+22*CELL, cfa+16*CELL);
    }

    DN_FORTH_WORD("COUNT", 
        // c-addr
        "1+", "DUP", "1-", "C@"
    );

    dn_create_variable("[TIB-CNT]", 0);
    dn_word_set_hidden();

    dn_create_variable("[TIB]", 0);
    dn_allot(100 - CELL);
    dn_word_set_hidden();
    // allocated space for SOURCE buffer


    dn_create_variable("[SRC]", 0);
    dn_word_set_hidden();

    dn_create_variable("[SRC-CNT]", 0);
    dn_word_set_hidden();

    dn_create_variable("[SRC-ID]", 0);
    dn_word_set_hidden();

    dn_create_variable(">IN", 0);

    DN_FORTH_WORD("SOURCE-ID", "[SRC-ID]", "@");

    {
        dn_cell_t cfa = (
            DN_FORTH_WORD("SOURCE", "SOURCE-ID", "[?JUMP]", (T_IMMEDIATE)0, "[SRC]", "@", "[SRC-CNT]", "@", "EXIT", "[TIB]", "[TIB-CNT]", "@")
        );
        dn_write_cell(cfa+2*CELL, cfa+8*CELL);
    }
/*
    DN_FORTH_WORD("SAVE-INPUT", 
        "[SRC]", "@",
        "[SRC-CNT]", "@",
        ">IN", "@",
        "[TIB-CNT]", "@",
        "SOURCE-ID"
    );
    DN_FORTH_WORD("RESTORE-INPUT", 
        "[SRC-ID]", "!",
        "[TIB-CNT]", "!",
        ">IN", "!",
        "[SRC-CNT]", "!",
        "[SRC]", "!"
    );
*/
    dn_create_system_word("REFILL", word_refill);

    DN_FORTH_WORD("[EMPTY?]",
        "SOURCE", "NIP",
        ">IN", "@",
        "="
    );

    {
        dn_cell_t cfa = (DN_FORTH_WORD("KEY", 
            "[EMPTY?]", "[?JUMP]", (T_IMMEDIATE)0,
                // empty -> try to refill
                "REFILL", "DROP",
                "[EMPTY?]", "[?JUMP]", (T_IMMEDIATE)0, 
                    -1, "EXIT",
            ">IN", "DUP", "@", "DUP",
            // (&in in in)
            "SOURCE", "DROP", "+", "C@",
            // (&in in val)
            "SWAP", "1+",
            // (&in val in+)
            "ROT", "!"
        ));
        dn_write_cell(cfa + 2*CELL, cfa + 11*CELL);
        dn_write_cell(cfa + 7*CELL, cfa + 11*CELL);
    }

    {
        dn_cell_t cfa = (
            DN_FORTH_WORD("ACCEPT",
                // addr cnt
                "SWAP", "OVER",
                // cnt addr cnt
                0, "[DOSET]", "[DO]", (T_IMMEDIATE)0,
                    // cnt addr
                    "KEY", 
                    // (cnt addr key)
                    "DUP", -1, "=", 
                    "[?JUMP]", (T_IMMEDIATE)0,
                        // (cnt addr key)
                        "2DROP", "DROP", "I", "UNLOOP", "EXIT",
                    // (cnt addr key)
                    "OVER", "I", "+", "C!",
                    // cnt addr
                "[LOOP]", (T_IMMEDIATE)0, "UNLOOP", "DROP"
            )
        );
        dn_write_cell(cfa+6*CELL, cfa + 25*CELL);
        dn_write_cell(cfa+24*CELL, cfa + 5*CELL);

        dn_write_cell(cfa+13*CELL, cfa + 19*CELL);
    }

    // PARSE
    // NOT STANDARD COMPILANT!
    // restult string NOT in input buffer!
    dn_create_variable("[PARSE]", 0);
    dn_word_set_hidden();

    dn_allot(100-CELL);
    {
        dn_cell_t cfa = (DN_FORTH_WORD("PARSE",
            // (delim)
            "[PARSE]", "SWAP", // (buff, delim)
            100, 0, "[DOSET]", "[DO]", (T_IMMEDIATE)0,
                // buff, edelim
                "KEY",
                // (buff, delim, key)

                "DUP", -1, "=",
                // (buff, delim, key, key==-1)
                "OVER", // buff, delim, key, key==-1, key
                3, "PICK", // buff, delim, key, key==-1, key, delim
                "=", "OR", "[?JUMP]", (T_IMMEDIATE)0,
                    // buff, delim, key
                    "2DROP", "I", "UNLOOP", "EXIT",
                // buff, delim, key
                2, "PICK", "I", "+", "C!",
            "[LOOP]", (T_IMMEDIATE)0, "UNLOOP", "DROP", -1
        ));
        dn_write_cell(cfa+8*CELL, cfa+34*CELL);
        dn_write_cell(cfa+33*CELL, cfa+7*CELL);

        dn_write_cell(cfa+21*CELL, cfa+26*CELL);
    }

    DN_FORTH_WORD("[IS-SPACE]",
        "DUP", ' ', "=", "INVERT",
        "OVER", '\n', "=", "INVERT", "AND"
    );
    dn_word_set_hidden();

    // PARSE-NAME
    // only care about spaces, NOT any whitespace
    // (IGNORE NEWLINES!)
    {
        dn_cell_t cfa = (
            DN_FORTH_WORD("PARSE-NAME",
                // skip spaces
                "KEY",
                "[IS-SPACE]",
                "[?JUMP]", (T_IMMEDIATE)0, // jump back to KEY               
                "[PARSE]", // key, addr
                "OVER",-1, "=", "[?JUMP]", (T_IMMEDIATE)0,
                    // got -1, EOF, ...
                    "DROP", 0, "EXIT",

                // parse it now
                "SWAP",
                100, 0, "[DOSET]", "[DO]", (T_IMMEDIATE)0,
                    // addr, key
                    "[IS-SPACE]", "INVERT",
                    // addr key space
                    "OVER", -1, "=", "OR",
                    "[?JUMP]", (T_IMMEDIATE)0,
                        // if space or EOF
                        "DROP", "I", "UNLOOP", "EXIT",
                    // addr key
                    "OVER", "I", "+", "C!",
                    "KEY",
                "[LOOP]", (T_IMMEDIATE)0, "UNLOOP", "DROP", -1
            )
        );
        dn_write_cell(cfa+3*CELL, cfa-CELL);

        dn_write_cell(cfa+10*CELL, cfa+15*CELL);
        dn_write_cell(cfa+22*CELL, cfa+41*CELL);
        dn_write_cell(cfa+42*CELL, cfa+21*CELL);
        dn_write_cell(cfa+31*CELL, cfa+36*CELL);
    }

    {
        dn_cell_t cfa = (
            DN_FORTH_WORD("WORD",
                "PARSE-NAME",// addr, cnt
                "DUP", "1+", 0, "[DOSET]", "[DO]", (T_IMMEDIATE)0,
                    // addr char-to-store
                    "OVER", "I", "+",
                    // addr char-to-store curr_pos
                    "DUP", "C@",
                    // addr char-to-store curr-pos curr-char
                    "ROT", "ROT",
                    // addr curr-char char-to-store curr-pos
                    "C!",
                "[LOOP]", (T_IMMEDIATE)0, "UNLOOP", "DROP"
            )
        );
        dn_write_cell(cfa + 7*CELL, cfa+18*CELL);
        dn_write_cell(cfa + 17*CELL, cfa+6*CELL);
    }

    DN_FORTH_WORD("CHAR",
        "PARSE-NAME", "DROP", "C@"
    );

    {
        dn_cell_t cfa = (
            DN_FORTH_WORD("[>DIGIT]",
                "DUP", '0', ('9'+1), "WITHIN", "[?JUMP]", (T_IMMEDIATE)0,
                    // single digit...
                    '0', "-", "[JUMP]", (T_IMMEDIATE)0,
                "DUP", 'a', ('z'+1), "WITHIN", "[?JUMP]", (T_IMMEDIATE)0,
                    ('a'-10), "-", "[JUMP]", (T_IMMEDIATE)0,
                "DUP", 'A', ('Z'+1), "WITHIN", "[?JUMP]", (T_IMMEDIATE)0,
                    ('A'-10), "-", "[JUMP]", (T_IMMEDIATE)0,
                // could not convert...
                "[JUMP]", (T_IMMEDIATE)0,
                // could convert, but is it OK with BASE?
                "DUP", "BASE", "@", "<", "[?JUMP]", (T_IMMEDIATE)0,
                    // it's OK...
                    "EXIT",
                "DROP", -1
            )
        );
        // ranges
        dn_write_cell(cfa+7*CELL, cfa+13*CELL);
        dn_write_cell(cfa+20*CELL, cfa+26*CELL);
        dn_write_cell(cfa+33*CELL, cfa+37*CELL);

        // jumps to check
        dn_write_cell(cfa+12*CELL, cfa+41*CELL);
        dn_write_cell(cfa+25*CELL, cfa+41*CELL);
        dn_write_cell(cfa+38*CELL, cfa+41*CELL);

        // exit jump
        dn_write_cell(cfa+40*CELL, cfa+48*CELL);

        dn_write_cell(cfa+46*CELL, cfa+48*CELL);

        dn_word_set_hidden();
    }

    {
        DN_FORTH_WORD("[STR+]", "1-", "SWAP", "1+", "SWAP");
        dn_word_set_hidden();

        dn_cell_t cfa = (
            DN_FORTH_WORD(">NUMBER",
                // (tot1 tot2 c-addr rem)

                "DUP", "0=", "INVERT", "[?JUMP]", (T_IMMEDIATE)0,
                    // grab char from c-addr
                    "OVER", "C@",
                    // tot1 tot2 c-addr rem CH
                    "[>DIGIT]", // convert to value
                    "DUP", -1, "=", "[?JUMP]", (T_IMMEDIATE)0,
                        // if returned -1, we have unconvertable char...
                        // tot1 tot2 c-addr rem -1
                        "DROP", "EXIT",
                    // we could convert it...
                    // (tot1 tot2 c-addr rem d)
                    ">R", "2SWAP", "BASE", "@", 0, "D*",
                    // ( c-addr rem tot1 tot2 ) (d)
                    "R>", 0, "D+",
                    // (c-addr rem tot1 tot2)
                    "2SWAP",
                    // (tot1 tot2 c-addr rem)
                    "[STR+]",
                    "[JUMP]", (T_IMMEDIATE)0
            )
        );
        dn_write_cell(cfa+4*CELL, cfa+31*CELL);
        dn_write_cell(cfa+13*CELL, cfa+16*CELL);
        dn_write_cell(cfa + 30*CELL, cfa);
    }

    {
        // DN_FORTH_WORD(">NUMBER", "cr", "BASE", "@","..S", "DROP","CR", ">number", "cr", 'A', "EMIT", ':', "EMIT", ".S","cr");
        dn_cell_t cfa = (
            DN_FORTH_WORD("NUMBER?",
                // (caddr cnt -- number? succ)
                "BASE", "@", "ROT", "ROT", // (base caddr cnt)
                "OVER", "C@", // (vase caddr cnt ch)
                "DUP", '#', "=", "[?JUMP]", (T_IMMEDIATE)0,
                    10, "BASE", "!", ">R", "[STR+]", "R>",
                "DUP", '$', "=", "[?JUMP]", (T_IMMEDIATE)0,
                    16, "BASE", "!", ">R", "[STR+]", "R>",
                "DUP", '%', "=", "[?JUMP]", (T_IMMEDIATE)0,
                    2, "BASE", "!", ">R", "[STR+]", "R>",
                '\'', "=", "[?JUMP]", (T_IMMEDIATE)0,
                    // must convert char...
                    "DROP", "1+", "C@", "NIP", -1, "EXIT", // val, -1
                // now base is set
                // (base addr cnt)
                // do we have a negative sign?
                "OVER", "C@", '-', "=", "2*", "1+", "ROT", "ROT",
                // (base sign addr cnt)
                2, "PICK", -1, "=", "[?JUMP]", (T_IMMEDIATE)0,
                    // need to advance...
                    "[STR+]",
                0, 0, "2SWAP", ">NUMBER",
                // (base sign nn nn addr rem)
                "[?JUMP]", (T_IMMEDIATE)0,
                    // convert error...
                    "2DROP", "2DROP", "BASE", "!", 0, 0, "EXIT",
                "2DROP", 
                // (base sign val)
                "M*", "DROP", 
                "SWAP", "BASE", "!", -1
            )
        );
        dn_write_cell(cfa + 11*CELL, cfa+19*CELL);
        dn_write_cell(cfa + 24*CELL, cfa+32*CELL);
        dn_write_cell(cfa + 37*CELL, cfa+45*CELL);
        
        dn_write_cell(cfa + 49*CELL, cfa+57*CELL);

        dn_write_cell(cfa + 73*CELL, cfa+75*CELL);

        dn_write_cell(cfa + 82*CELL, cfa+92*CELL);

    }
}