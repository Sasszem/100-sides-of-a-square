#include "forth.h"
#include <stdio.h>

static void _fancy_print_hex_cell(dn_cell_t data) {
    printf("\e[2m");
    int mode = 1;
    for (int i = 0; i<4; i++) {
        uint8_t val = (data >> (24-8*i)) & 0xff;
        if (val && mode) {
            printf("\e[22m");
            mode = 0;
        }
        printf("%02x", val);
    }
    if (mode)
        printf("\e[22m");
}

static void _fancy_print_ascii_cell(dn_cell_t data) {
    for (int i = 0; i<4; i++) {
        uint8_t val = (data >> (24-8*i)) & 0xff;
        if (val >= 32 && val<= 126) {
            putchar(val);
        } else {
            printf("\e[100m \e[49m");
        }
    }
}


void _print_cell_address_content(dn_cell_t addr) {
    printf("%05x\t", addr);
    _fancy_print_hex_cell(dn_read_cell(addr));
}

void dn_disas_word(dn_cell_t xt) {
    printf("\n");
    uint32_t ht = 37+dn_word_get_name_len(xt);
    for(int i = 0; i<ht; i++) putchar('#'); putchar('\n');
    printf("# Disassembling word at %05xh - '%.*s' #\n", xt, dn_word_get_name_len(xt), dn_word_get_name_ptr(xt));
    for(int i = 0; i<ht; i++) putchar('#'); putchar('\n');
    printf("\n\e[4mHEADER:\e[24m\n");
    
    _print_cell_address_content(xt);
    dn_cell_t header = dn_read_cell(xt);
    printf("\n");
    _print_cell_address_content(xt+CELL);
    printf("\n");
    

    _print_cell_address_content(xt+2*CELL);
    printf("\n\n");
    printf("Link field ptr: %05x ('%.*s')\n", dn_word_get_prev_ptr_from_word(xt), dn_word_get_name_len(dn_word_get_prev_ptr_from_word(xt)), dn_word_get_name_ptr(dn_word_get_prev_ptr_from_word(xt)));
    printf("Name len: %d\n", dn_word_get_name_len(xt));

    if (header & (_WORD_IS_IMMEDIATE | _WORD_IS_PRIMITIVE | _WORD_IS_SKIP | _WORD_IS_HIDDEN))
        printf("\nAttributes:\n");
    if (header & _WORD_IS_IMMEDIATE) 
        printf("- IMMIDIATE\n");
    if (header & _WORD_IS_PRIMITIVE)
        printf("- PRIMITIVE (%p)\n", dn_word_get_fptr_from_xt(xt));
    if (header & _WORD_IS_SKIP)
        printf("- SKIP\n");
    if (header & _WORD_IS_HIDDEN)
        printf("- HIDDEN\n");

    printf("\n");

    dn_cell_t pfa = dn_read_cell(xt + CELL);
    dn_cell_t cfa = dn_read_cell(xt + 2*CELL);
    if (~header & _WORD_IS_PRIMITIVE) {
        printf("PFA: %05x\n", pfa);
        printf("CFA: %05x\n", cfa);   

        if (pfa != cfa) {
            printf("PFA != CFA, this word might contain data!\n");
        } else {
            printf("PFA = CFA, this word only contains code!\n");
        }
        if (cfa && cfa < xt) {
            dn_cell_t cfa_word = dn_lookup_word_containing_address(cfa);
            printf("CFA points into word '%.*s' (%05x)\n", dn_word_get_name_len(cfa_word), dn_word_get_name_ptr(cfa_word), cfa_word);
        }
    }

    printf("\n\e[4mNAME AREA:\e[24m\n");

    for (int n = 0; n<dn_word_get_name_len(xt); n+=4) {
        dn_cell_t cell_addr = xt+n+ 3*CELL;
        dn_cell_t cell_val = dn_read_cell(cell_addr);
        printf("%05x\t", cell_addr);

        _fancy_print_hex_cell(cell_val);
        printf("\t");
        _fancy_print_ascii_cell(cell_val);
        printf("\n");
    }

    if (header & _WORD_IS_PRIMITIVE)
        return;

    if (pfa && pfa!=cfa) {
        printf("\n\e[4mPARAMETER FIELD:\e[24m\n");

        dn_cell_t next_word_ptr = dn_next_word_after_address(xt);
        dn_cell_t cfa_limit = (cfa < pfa) ? next_word_ptr : cfa; // disable limit if CFA is smaller
        // printf("Next word loc returned is %05x ('%.*s')\n", next_word_ptr, dn_word_get_name_len(next_word_ptr), dn_word_get_name_ptr(next_word_ptr));
        for (dn_cell_t pfa_ptr = pfa; pfa_ptr < next_word_ptr && pfa_ptr < cfa_limit; pfa_ptr += CELL) {
            printf("%05x\t", pfa_ptr);
            _fancy_print_hex_cell(dn_read_cell(pfa_ptr));
            printf("\n");
        }
    }

    if(cfa) {
        printf("\n\e[4mCODE FIELD:\e[24m\n");

        if (cfa > dn_next_word_after_address(xt)) {
            dn_cell_t defword = dn_lookup_word_containing_address(cfa);
            printf("(Defined in '%.*s')\n", dn_word_get_name_len(defword), dn_word_get_name_ptr(defword));
        }
        
        dn_cell_t next_word_ptr = dn_next_word_after_address(cfa);
        // printf("Next_word returned %05x '%.*s'\n", next_word_ptr, dn_word_get_name_len(next_word_ptr), dn_word_get_name_ptr(next_word_ptr));
        for (dn_cell_t cfa_ptr = cfa; cfa_ptr < next_word_ptr; cfa_ptr += CELL) {
            printf("%05x (%+3d)\t", cfa_ptr, (cfa_ptr - cfa)/CELL -1);
            dn_cell_t token = dn_read_cell(cfa_ptr);
            _fancy_print_hex_cell(token);
            printf("\t");
            _fancy_print_ascii_cell(token);
            printf("\t%.*s\n",dn_word_get_name_len(token), dn_word_get_name_ptr(token));
        }
    }
}


void dn_dump_cells(dn_cell_t start, dn_cell_t count) {
    dn_cell_t address = dn_mask_address(start);
    for (dn_cell_t i = 0; i<count; i++) {
        dn_cell_t cell_start_address = address + i*CELL;
        dn_cell_t cell_value = dn_read_cell(cell_start_address);

        printf("%05x\t", cell_start_address);
        _fancy_print_hex_cell(cell_value);
        printf("\t");
        _fancy_print_ascii_cell(cell_value);
        printf("\n");
    }
}

void dn_dump_memory(dn_cell_t start_address, dn_cell_t len) {
    dn_cell_t row_address = dn_mask_address(start_address) - start_address%8;
    while (row_address <= dn_mask_address(start_address + len)) {
        printf("%05x\t", row_address);
        for (int i = 0; i<8; i++) {
            printf("%02x ", DN_FORTH_MEMORY[row_address + i]);
        }
        printf("\n");
        row_address += 8;
    }
}


void dn_debug_dump_stack() {
    dn_cell_t debth = (dn_get_var("S0") - dn_get_var("SP@")) / 4;
    printf("(%d) ", debth);
    for (; debth > 0; debth--) {
        printf("%d ", dn_read_cell(dn_get_sp()+CELL*debth));
    }
    printf("\n");
}


void dn_debug_dump_ret_stack() {
    dn_cell_t depth = (dn_get_var("R0") - dn_get_var("RP@")) / 4;
    printf("(%dr) ", depth);
    for (; depth > 0; depth--) {
        printf("%x ", dn_read_cell(dn_get_var("RP@")+CELL*depth));
    }
    printf("\n");
}


void dn_print_backtrace() {
    dn_rpush(dn_get_var("[IP]"));
    dn_rpush(dn_get_var("[xt]"));
    dn_cell_t depth = (dn_get_var("R0") - dn_get_var("RP@")) / 4;
    printf("BACKTRACE (most recent call first, showing return address):\n");
    for (int i = 0; i<depth; i++) {
        dn_cell_t addr = dn_rpeek(i);
        dn_cell_t xt = dn_lookup_word_containing_address(addr);
        if (addr)
            printf("- %05x ('%.*s')\n", addr, dn_word_get_name_len(xt), dn_word_get_name_ptr(xt));
    }
    dn_rpop(); dn_rpop();
}

