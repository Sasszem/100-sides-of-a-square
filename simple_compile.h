#include "forth.h"
#include "map.h"

#pragma clang diagnostic ignored "-Wcompound-token-split-by-macro"

#define compile_token(x) _Generic((x), \
    int: dn_compile_literal((dn_cell_t)x),  \
    dn_cell_t: dn_compile_literal((dn_cell_t)x), \
    char*: dn_compile((char*)x), \
    T_IMMEDIATE: dn_allot_write((dn_cell_t)x) \
);

#define _COMPILE_FORTH_TOKENS(...) \
_Pragma("GCC diagnostic push") \
_Pragma("GCC diagnostic ignored \"-Wpointer-to-int-cast\"") \
_Pragma("GCC diagnostic ignored \"-Wint-to-pointer-cast\"") \
MAP(compile_token, __VA_ARGS__) \
_Pragma("GCC diagnostic pop")


#define DN_FORTH_WORD(name, ...) {dn_cell_t cfa = dn_start_word(name); _COMPILE_FORTH_TOKENS(__VA_ARGS__); dn_finish_compiled_word();cfa;}
