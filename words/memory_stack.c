#include "../forth.h"
#include "../simple_compile.h"

/**
    Word "@" - fetch cell
    
    (a-addr -- x)

    https://forth-standard.org/standard/core/Fetch
*/
static void word_mem_fetch() {
    dn_push(dn_read_cell(dn_pop()));
}

/**
    Word "!" - store cell

    (x a-addr --)

    https://forth-standard.org/standard/core/Store
*/
static void word_mem_store() {
    dn_cell_t addr = dn_pop();
    dn_cell_t x = dn_pop();
    dn_write_cell(addr, x);
}

/**
    Word "C!" - store character

    (char c-addr -- )
    https://forth-standard.org/standard/core/CStore
*/
static void word_mem_cfetch() {
    dn_push(dn_read_char(dn_pop()));
}

/**
    Word "C@" - fetch char

    (c-addr -- char)

    https://forth-standard.org/standard/core/CFetch
*/
static void word_mem_cstore() {
    dn_cell_t addr = dn_pop();
    dn_cell_t x = dn_pop();
    dn_write_char(addr, x);
}

/**
    Word "DROP" - discard top of stack

    (x -- )

    https://forth-standard.org/standard/core/DROP
*/
static void word_drop() {
    dn_pop();
}

/**
    Word "DUP" - duplicate top of stack

    (x -- x x)

    https://forth-standard.org/standard/core/DUP
*/
static void word_dup() {
    dn_cell_t val = dn_pop();
    dn_push(val);
    dn_push(val);
}

/**
    Word "PICK" - Pick element from stack

    (... x3 x2 x1 x0 n -- x3 x2 x1 x0 xn)

    https://forth-standard.org/standard/core/PICK
*/
static void word_pick() {
    dn_push(dn_peek(dn_pop()));
}

/**
    Word "SWAP" - Swap top elements of stack

    (x2 x1 -- x1 x2)

    https://forth-standard.org/standard/core/SWAP
*/
static void word_swap() {
    dn_cell_t a = dn_pop();
    dn_cell_t b = dn_pop();

    dn_push(a);
    dn_push(b);
}

/**
    Word "ROT" - rotate top 3 elements of stack

    (x1 x2 x3 -- x2 x3 x1)

    https://forth-standard.org/standard/core/ROT
*/
static void word_rot() {
    dn_cell_t x3 = dn_pop();
    dn_cell_t x2 = dn_pop();
    dn_cell_t x1 = dn_pop();

    dn_push(x2);
    dn_push(x3);
    dn_push(x1);
}

/**
    Word "R@" - get top of return stack

    (-- x) (R: x -- )

    https://forth-standard.org/standard/core/RFetch
*/
static void word_r_fetch() {
    dn_push(dn_rpeek(0));
}

/**
    Word "R>" - move from return stack

    (-- x) (R: x -- )

    https://forth-standard.org/standard/core/Rfrom
*/
static void word_r_from() {
    dn_push(dn_rpop());
}

/**
    Word ">R" - move to return stack

    (x -- ) (R: -- x)

    https://forth-standard.org/standard/core/toR
*/
static void word_to_r() {
    dn_rpush(dn_pop());
}

/**
    Word "ALLOT" - allocate characters

    (x -- )

    https://forth-standard.org/standard/core/ALLOT
*/
static void word_allot() {
    dn_allot(dn_pop());
}

/**
    Word "S>D" - single to double conversion

    (n -- d)

    https://forth-standard.org/standard/core/StoD
*/
static void word_s_to_d() {
    int64_t v = (int64_t) dn_pop();
    dn_push(v & (0xffffffff));
    dn_push((v>>32) & (0xffffffff));
}

/**
    Word "ALIGN" - align HERE by reserving space

    (--)

    https://forth-standard.org/standard/core/ALIGN
*/
static void word_align() {
    dn_align_here();
}

/**
    Word "ALIGNED" - get next cell-aligned address

    (--)

    https://forth-standard.org/standard/core/ALIGNED
*/
static void word_aligned() {
    dn_cell_t addr = dn_pop();
    dn_cell_t rem = addr % CELL;
    
    dn_push(rem ? addr + (4-rem) : addr);
}

void words_set_stack_memory() {
    dn_create_system_word("@", word_mem_fetch);
    dn_create_system_word("!", word_mem_store);
    dn_create_system_word("C@", word_mem_cfetch);
    dn_create_system_word("C!", word_mem_cstore);
    dn_create_system_word("ALLOT", word_allot);
    dn_create_system_word("ALIGN", word_align);
    dn_create_system_word("ALIGNED", word_aligned);

    dn_create_system_word("PICK", word_pick);
    dn_create_system_word("SWAP", word_swap);
    dn_create_system_word("ROT", word_rot);
    dn_create_system_word("DROP", word_drop);

    dn_create_system_word("R@", word_r_fetch);
    dn_create_system_word("R>", word_r_from);
    dn_create_system_word(">R", word_to_r);
    dn_create_system_word("S>D", word_s_to_d);
}

void words_set_derived_stack_memory() {
    DN_FORTH_WORD("DUP", 0, "PICK");
    DN_FORTH_WORD("OVER", 1, "PICK");
    DN_FORTH_WORD("TUCK", "SWAP", "OVER");
    DN_FORTH_WORD("NIP", "SWAP", "DROP");
    DN_FORTH_WORD("DEPTH", "S0", "@", "SP@", "-", 4, "/", "1-");
    DN_FORTH_WORD("CELLS", 4, "*");
    DN_FORTH_WORD("CHARS", "EXIT");
    DN_FORTH_WORD("CELL+", 4, "+");
    DN_FORTH_WORD("2>R", "R>", "ROT", ">R", "SWAP", ">R", ">R");
    DN_FORTH_WORD("2R>", "R>", "R>", "R>", "ROT", ">R", "SWAP");
    DN_FORTH_WORD("2R@", "R>", "R>", "R@", "ROT", "ROT", "DUP", ">R", "SWAP", ">R");
    DN_FORTH_WORD("2DUP", "OVER", "OVER");
    DN_FORTH_WORD("2DROP", "DROP", "DROP");

    {
        dn_cell_t cfa = (
            DN_FORTH_WORD("?DUP", "DUP", "[?JUMP]", (T_IMMEDIATE)0, "DUP"
        ));
        dn_write_cell(cfa+2*CELL, cfa+4*CELL);
    }

    {
        dn_cell_t cfa = (
            DN_FORTH_WORD("ROLL", "DUP", 0, "=", "[?JUMP]", (T_IMMEDIATE)0, "DROP", "EXIT", "SWAP", ">R", "1-", (T_IMMEDIATE)1, "R>", "SWAP"
        ));
        dn_write_cell(cfa+5*CELL, cfa+8*CELL);
        dn_write_cell(cfa+11*CELL, dn_lookup_word("ROLL"));
    }
}

void words_set_derived_2_stack_memory() {
    {
        dn_cell_t cfa = (
            DN_FORTH_WORD("FILL", 
                // (c-addr u char --)
                "SWAP", 0, "MAX", 0, // (c-addr char limit 0)
                "[DOSET]", "[DO]", (T_IMMEDIATE)0,
                    // (c-addr char)
                    "DUP", 2, "PICK", "I", "+", "C!",
                "[LOOP]", (T_IMMEDIATE)0, "UNLOOP", "2DROP"
            )
        );
        dn_write_cell(cfa + 8*CELL, cfa+18*CELL);
        dn_write_cell(cfa + 17*CELL, cfa+7*CELL);
    }

    DN_FORTH_WORD("ERASE", 0, "FILL");

    {
        dn_cell_t cfa = (
            DN_FORTH_WORD("MOVE", 
                // (src dst cnt --)
                0, "MAX", 0, // (src dst limit 0)
                "[DOSET]", "[DO]", (T_IMMEDIATE)0,
                    // (src dst)
                    "OVER", "I", "+", "C@", // (src dst S)
                    "OVER", "I", "+", "C!",
                "[LOOP]", (T_IMMEDIATE)0, "UNLOOP", "2DROP"
            )
        );
        dn_write_cell(cfa + 7*CELL, cfa+18*CELL);
        dn_write_cell(cfa + 17*CELL, cfa+6*CELL);
    }

    DN_FORTH_WORD("UNUSED", "SP@", "HERE", "-");
}