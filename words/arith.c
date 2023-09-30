#include "forth_words.h"
#include "../simple_compile.h"
#include <stdint.h>

/**
    Word "+" - single cell addition

    (a b -- a+b)

    https://forth-standard.org/standard/core/Plus
*/
static void word_plus() {
    dn_push(dn_pop() + dn_pop());
}

/**
    Word "-" - single cell subtraction

    (a b -- a-b)

    https://forth-standard.org/standard/core/Minus
*/
static void word_minus() {
    dn_cell_t a = dn_pop();
    dn_cell_t b = dn_pop();

    dn_push(b - a);
}

/**
    Word "/" - single cell division

    (a b -- a/b)

    https://forth-standard.org/standard/core/Div

    Returns -1 if b is 0
*/
static void word_div() {
    dn_cell_t a = dn_pop();
    dn_cell_t b = dn_pop();
    if (!a)
        dn_abort("ZERODIV");

    dn_push(a!=0 ? b / a : -1);
}

/**
    Word "*" - single cell multiplication

    (a b -- a*b)

    https://forth-standard.org/standard/core/Times
*/
static void word_times() {
    dn_push(dn_pop()*dn_pop());
}

/**
    Word "\*\/" - multply and divide with double precision

    (a b c -- a*b/c)

    https://forth-standard.org/standard/core/TimesDiv

    Return -1 if c is 0
*/
static void word_timesdiv() {
    dn_cell_t n3 = dn_pop();
    dn_cell_t n2 = dn_pop();
    dn_cell_t n1 = dn_pop();
    if (n3)
        dn_push((uint64_t)(n2) * (uint64_t)(n1) / (uint64_t)(n3));
    else {
        dn_abort("ZERODIV");
        dn_push(-1);
    }
}

/**
    Word "/MOD" - divide and modulo

    (a b -- a%b a/b)

    https://forth-standard.org/standard/core/DivMOD

    Return (-1 -1) if b is 0
*/
static void word_divmod() {
    dn_cell_t n2 = dn_pop();
    dn_cell_t n1 = dn_pop();
    if (!n2)         
        dn_abort("ZERODIV");
    dn_push(n2 ? n1 % n2 : -1);
    dn_push(n2 ? n1 / n2 : -1);
}

/**
    Word "\*\/MOD" - multiply, divide and modulo

    (a b c -- a*b%c a*b/c)

    https://forth-standard.org/standard/core/TimesDivMOD

    Return (-1 -1) if c is 0
*/
static void word_timesdivmod() {
    int32_t n3 = dn_pop();
    dn_cell_t n2 = dn_pop();
    dn_cell_t n1 = dn_pop();
    if (n3) {
        int64_t tmp = (int64_t)n1 * (int64_t)n2;
        dn_push(tmp % n3);
        dn_push(tmp / n3);
    } else {
        dn_abort("ZERODIV");
        dn_push(-1);
        dn_push(-1);
    }
}

/**
    Word "UM/MOD" - unsigned double divmod
    
    (ud u -- ud%u ud/u)

    https://forth-standard.org/standard/core/UMDivMOD

    Returns (-1 -1) if u is 0. Return values are single-cell
*/
static void word_umdivmod() {
    dn_cell_t u1 = dn_pop();
    dn_cell_t ud1 = dn_pop();
    dn_cell_t ud2 = dn_pop();
    uint64_t ud = (uint64_t)ud2 + ((uint64_t)ud1<<32);

    if (!u1) {
        dn_abort("ZERODIV");
        dn_push(-1);
        dn_push(-1);
        return;
    }

    dn_cell_t u3 = ud / u1;
    dn_cell_t u2 = ud % u1;

    dn_push(u2);
    dn_push(u3);
}

/**
    Word "UM/MOD2" - unsigned double divmod with double quotient
    
    (ud u -- ud%u (double)ud/u)

    Returns (-1 -1) if u is 0. Quotient is double, remainder is single cell.
    Used by word "#".
*/
static void word_umdivmod2() {
    // just like umdivmod, but quotient u3 is also double
    dn_cell_t u1 = dn_pop();
    dn_cell_t ud1 = dn_pop();
    dn_cell_t ud2 = dn_pop();
    uint64_t ud = (uint64_t)ud2 + ((uint64_t)ud1<<32);

    if (!u1) {
        dn_abort("ZERODIV");
        dn_push(-1);
        dn_push(-1);
        dn_push(-1);
        return;
    }

    uint64_t u3 = ud / u1;
    dn_cell_t u2 = ud % u1;

    dn_push(u2);
    dn_push(u3 & (0xffffffff));
    dn_push((u3>>32) & (0xffffffff));
}


/**
    Word "M*" - signed double product
    
    (n1 n2 -- (double)n1*n2)

    https://forth-standard.org/standard/core/MTimes
*/
static void word_mtimes() {
    int32_t n2 = dn_pop();
    int32_t n1 = dn_pop();

    int64_t prod = (int64_t)n1 * (int64_t)n2;

    dn_push(prod & (0xffffffff));
    dn_push((prod>>32) & (0xffffffff));
}

/**
    Word "UM*" - unsigned double product
    
    (u1 u2 -- (double)u1*u2)

    https://forth-standard.org/standard/core/UMTimes
*/
static void word_umtimes() {
    uint32_t u2 = dn_pop();
    uint32_t u1 = dn_pop();

    uint64_t prod = (int64_t)u1 * (int64_t)u2;

    dn_push(prod & (0xffffffff));
    dn_push((prod>>32) & (0xffffffff));
}

/**
    Word "SM/REM" - signed double divmod

    (nd n -- nd%n nd/n)

    Returns (-1 -1) if n is 0.

    https://forth-standard.org/standard/core/SMDivREM
*/
static void word_smdivrem() {
    int32_t n1 = dn_pop();
    int32_t nd1 = dn_pop();
    int32_t nd2 = dn_pop();
    int64_t nd = (int64_t)nd2 + ((int64_t)nd1<<32);

    if (!n1) {
        dn_abort("ZERODIV");
        dn_push(-1);
        dn_push(-1);
        return;
    }

    int32_t n3 = nd / n1;
    int32_t n2 = nd % n1;

    dn_push(n2);
    dn_push(n3);
}

/**
    Word "<" - less than

    (x1 x2 -- (flag)x1<x2)

    https://forth-standard.org/standard/core/less
*/
static void word_less() {
    int32_t n2 = dn_pop();
    int32_t n1 = dn_pop();
    
    dn_push(n1 < n2 ? (-1) : 0);
}

/**
    Word ">" - more than

    (x1 x2 -- (flag)x1>x2)

    https://forth-standard.org/standard/core/less
*/
static void word_more() {
    int32_t n2 = dn_pop();
    int32_t n1 = dn_pop();
    
    dn_push(n1 > n2 ? (-1) : 0);
}

/**
    Word "U<" - unsigned less than

    (u1 u2 -- (flag)u1<u2)

    https://forth-standard.org/standard/core/Uless
*/
static void word_uless() {
    uint32_t n2 = dn_pop();
    uint32_t n1 = dn_pop();
    
    dn_push(n1 < n2 ? (-1) : 0);
}

/**
    Word "U>" - unsigned more than

    (u1 u2 -- (flag)u1>u2)

    https://forth-standard.org/standard/core/Uless
*/
static void word_umore() {
    uint32_t n2 = dn_pop();
    uint32_t n1 = dn_pop();
    
    dn_push(n1 > n2 ? (-1) : 0);
}
 
/**
    Word "AND" - biwise and

    (x1 x2 -- x1&x2)

    https://forth-standard.org/standard/core/AND
*/
static void word_and() {
    dn_push(dn_pop() & dn_pop());
}

/**
    Word "OR" - biwise or

    (x1 x2 -- x1|x2)

    https://forth-standard.org/standard/core/OR
*/
static void word_or() {
    dn_push(dn_pop() | dn_pop());
}

/**
    Word "XOR" - biwise xor

    (x1 x2 -- x1^x2)

    https://forth-standard.org/standard/core/XOR
*/
static void word_xor() {
    dn_push(dn_pop() ^ dn_pop());
}

/**
    Word "INVERT" - biwise invert

    (x1 -- ~x1)

    https://forth-standard.org/standard/core/INVERT
*/
static void word_invert() {
    dn_push(~dn_pop());
}

/**
    Word "LSHIFT" - logical left shift

    (x1 u -- x1<<u)

    https://forth-standard.org/standard/core/LSHIFT
*/
static void word_lshift() {
    dn_cell_t u = dn_pop();
    dn_cell_t x = dn_pop();
    dn_push(x<<u);
}

/**
    Word "RSHIFT" - logical right shift

    (x1 u -- x1<<u)

    https://forth-standard.org/standard/core/RSHIFT
*/
static void word_rshift() {
    dn_cell_t u = dn_pop();
    dn_cell_t x = dn_pop();
    dn_push(x>>u);
}

/**
    Word "=" - equality comparison

    (x1 x2 -- (flag)x1=x2)

    https://forth-standard.org/standard/core/Equal
*/
static void word_eq() {
    dn_push(dn_pop() == dn_pop() ? -1 : 0);
}

static void word_dmul() {
    // (d21 d22 d11 d12)
    int32_t d12 = dn_pop();
    int32_t d11 = dn_pop();
    int64_t d1 = (int64_t)d11 + ((int64_t)d12<<32);
    int32_t d22 = dn_pop();
    int32_t d21 = dn_pop();
    int64_t d2 = (int64_t)d21 + ((int64_t)d22<<32);

    int64_t m = d2 * d1;
    dn_push(m & 0xffffffff);
    dn_push((m >> 32) & 0xffffffff);
}


static void word_dadd() {
    // (d21 d22 d11 d12)
    int32_t d12 = dn_pop();
    int32_t d11 = dn_pop();
    int64_t d1 = (int64_t)d11 + ((int64_t)d12<<32);
    int32_t d22 = dn_pop();
    int32_t d21 = dn_pop();
    int64_t d2 = (int64_t)d21 + ((int64_t)d22<<32);

    int64_t m = d2 + d1;
    dn_push(m & 0xffffffff);
    dn_push((m >> 32) & 0xffffffff);
}

void words_set_arith() {
    dn_create_system_word("+", word_plus);
    dn_create_system_word("-", word_minus);
    dn_create_system_word("/", word_div);
    dn_create_system_word("*", word_times);
    dn_create_system_word("*/", word_timesdiv);
    dn_create_system_word("/MOD", word_divmod);
    dn_create_system_word("*/MOD", word_timesdivmod);
    dn_create_system_word("SM/REM", word_smdivrem);
    dn_create_system_word("UM/MOD", word_umdivmod);
    dn_create_system_word("UM/MOD2", word_umdivmod2);
    dn_create_system_word("M*", word_mtimes);
    dn_create_system_word("UM*", word_mtimes);

    dn_create_system_word("D*", word_dmul);
    dn_create_system_word("D+", word_dadd);

    dn_create_system_word("AND", word_and);
    dn_create_system_word("OR", word_or);
    dn_create_system_word("XOR", word_xor);
    dn_create_system_word("INVERT", word_invert);


    dn_create_system_word("LSHIFT", word_lshift);
    dn_create_system_word("RSHIFT", word_rshift);

    dn_create_system_word("=", word_eq);
    dn_create_system_word("<", word_less);
    dn_create_system_word(">", word_more);
    dn_create_system_word("U<", word_uless);
    dn_create_system_word("U>", word_umore);

    // technicly derived, but other words depend on thse 2, and they only depend on DROP and [LITERAL]
    DN_FORTH_WORD("1+", 1, "+");
    DN_FORTH_WORD("1-", 1, "-");
}

void words_set_derived_arith() {
    DN_FORTH_WORD("0=", 0, "=");
    DN_FORTH_WORD("0<>", 0, "=", "INVERT");
    DN_FORTH_WORD("0<", 0, "<");
    DN_FORTH_WORD("0>", 0, ">");
    DN_FORTH_WORD("<>", "=", "INVERT");
    DN_FORTH_WORD("FALSE", 0);
    DN_FORTH_WORD("TRUE", -1);

    DN_FORTH_WORD("CHAR+", "1+");

    DN_FORTH_WORD("2!", "SWAP", "OVER", "!", "CELL+", "!");
    DN_FORTH_WORD("2@", "DUP", "CELL+", "@", "SWAP", "@");
    DN_FORTH_WORD("2OVER", 3, "PICK", 3, "PICK");
    DN_FORTH_WORD("2SWAP", "ROT", ">R", "ROT", "R>");

    DN_FORTH_WORD("2*", 2, "*");
    DN_FORTH_WORD("2/", "DUP", 1, "RSHIFT", "SWAP", (1<<31), "AND", "OR");


    DN_FORTH_WORD("MOD",
        "/MOD", "DROP"
    );

    DN_FORTH_WORD("NEGATE", 0, "SWAP", "-");

    DN_FORTH_WORD("+!", 
        "SWAP", "OVER", "@", "+", "SWAP", "!"
    );

    {
        dn_cell_t cfa = (DN_FORTH_WORD("MAX", "2DUP", "<", "[?JUMP]", (T_IMMEDIATE)0, "SWAP", "DROP"));
        dn_write_cell(cfa + 3*CELL, cfa+5*CELL);
    }
    {
        dn_cell_t cfa = (DN_FORTH_WORD("MIN", "2DUP", ">", "[?JUMP]", (T_IMMEDIATE)0, "SWAP", "DROP"));
        dn_write_cell(cfa + 3*CELL, cfa+5*CELL);
    }
    {
        dn_cell_t cfa = (DN_FORTH_WORD("ABS", "DUP", (1<<31),"AND", "[?JUMP]", (T_IMMEDIATE)0, 0, "SWAP", "-"));
        dn_write_cell(cfa + 5*CELL, cfa+10*CELL);
    }

    DN_FORTH_WORD("WITHIN", "OVER", "-", ">R", "-", "R>", "U<");
}