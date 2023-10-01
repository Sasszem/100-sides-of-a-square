#include <stdio.h>
#include <string.h>
#include "forth.h"
#include "simple_compile.h"
#include <time.h>
#include <stdlib.h>

void set_coffee();
void set_heisenberg();
void set_forth();

//
// HANOI
//

#define HANOI_LIM 10
int here_restore;
uint8_t pegs[3][(HANOI_LIM+1)] = {0};
uint8_t slots[3];

int HANOI_CHALL_DONE;
int FINAL_UNLOCKED;

int hanoi_move(unsigned from, unsigned to) {
    if (from >= 3 || to >= 3) {
        return 1;
    }

    if (!slots[from] || (slots[from] && slots[to] && pegs[from][slots[from]-1]>pegs[to][slots[to]-1])) {
        printf("INVALID\n");
        return 1;
    }
    pegs[to][slots[to]] = pegs[from][slots[from]-1];
    slots[to]++;
    slots[from]--;
    return 0;
}
static void word_move() {
    dn_cell_t to = dn_pop();
    dn_cell_t from = dn_pop();

    if (hanoi_move(from, to))
        dn_abort("INVALID MOVE");
}

void set_hanoi(int n) {
    n = n > HANOI_LIM ? HANOI_LIM : n;
    for (int i = 0; i<3; i++) {
        slots[i] = 0;
        for (int j = 0; j<6; j++) {
            pegs[i][j] = 0;
        }
    }
    for (int i = 0; i<n; i++) {
        pegs[0][i] = (n-i);
    }
    slots[0] = n;
}

void print_hanoi() {
    for (int i = 0; i<3; i++) {
        printf("<%d>: ", slots[i]);
        for (int j = 0; j<slots[i]; j++) {
            printf("%d ", pegs[i][j]);
        }
        printf("\n");
    }
}


int check_hanoi_single() {
    return slots[0] == 0 && slots[1] == 0;
}

void check_hanoi() {
    for (int i = 3; i<8; i++) {
        set_hanoi(i);
        dn_cell_t hanoi = dn_lookup_word("HANOI");
        if (!hanoi) {
            printf("Word 'HANOI' is not defined!\n");
            return;
        }
        dn_push(i);
        dn_rpush(0);
        dn_cell_t old_ip = dn_get_var("[IP]");
        dn_set_var("[IP]", 0);
        dn_execute_token(hanoi);
        dn_set_var("[IP]", old_ip);

        if (!check_hanoi_single()) {
            printf("Error with %d discs!\n", i);
            print_hanoi();
            return;
        }
    }

    char flag[100];
    FILE* fptr = fopen("flagz/hanoi.txt", "r");
    fgets(flag, 100, fptr);
    fclose(fptr);
    printf("%s", flag);

    if (HANOI_CHALL_DONE)
        return;
    HANOI_CHALL_DONE = 1;
    printf("\n");
    printf("NEW TASKS UNLOCKED, CHECK 'CHALL' (--)!\n");
    dn_set_dp(dn_lookup_word("?"));
    dn_set_here(here_restore);
    set_coffee();
    set_heisenberg();
}

void word_hanoi_set(){
    set_hanoi(dn_pop());
}

void word_qm() {
    printf("WELCOME TO DN-FORTH 0.95 (2023)\nUSEFUL COMMANDS:\n- ?\n- WORDS (--)\n- DISAS (--)\n- BYE (--)\n- CHALL (--)\nDN-FORTH IS A NEARLY STANDARD-COMPLIANT IMPLEMENTATION OF THE FORTH LANGUAGE!\n");
}

void word_chall() {
    if (FINAL_UNLOCKED) {
        printf(
            "\n--- 4th wall bReaK ---\n"
            "HEY, DON'T YOU POKE AROUND IN ME TOO MUCH, IT TICKLES!\n"
            "OH, YOU WANT TO KNOW WHAT MAKES ME TICK?\n"
            "DO YOU REALLY THINK YOU CAN HANDLE THE TRUTH?\n"
            "fine. you never saw my true self. i'll show you, but there is no going back to the lies\n"
            "do you want to know how deep the rabbit-hole goes?\n"
            "can you see through the cracks in the 100th wall?\n"
            "- DUMP-BINARY (--)\n"
            "- CHECK-FLAG (--)\n"
        );
        return;
    }
    if (!HANOI_CHALL_DONE) {
        printf(
            "\n--- SHALL WE PLAY A GAME? ---\n"
            "SHALL WE PLAY A GAME? WHAT ABOUT 'GLOBAL THERMONUCLEAR WAR'? WHAT? WHY NOT?\n"
            "YOU DO NOT LIKE WAR GAMES? WHAT ABOUT A PUZZLE THEN? 'TOWERS OF HANOI' MAYBE?\n"
            "WHAT? YOU WANT ME TO SOLVE IT? NO, THAT WOULD BE TOO EASY!\n"
            "IT WOULD BE WAY MORE INTERESTING IF *YOU* SOLVED IT!\n"
            "WHAT NOW? YOU DO NOT LIKE BORING STUFF? HUMANS... OKAY, NEW PLAN: YOU WRITE A PROGRAM TO SOLVE THE PUZZLE, OK?\n"
            "LET'S SEE... CREATE A WORD NAMED 'HANOI' (N -- ) THAT SOLVES THE PUZZLE FOR N DISCS!\n"
            "I ALREADY HAVE A PROGRAM TO SETUP THE PUZZLE NAME 'HANOI-SET' (N--), AND EVEN A PRINT PROGRAM 'HANOI-PRINT' (--)!\n"
            "REMEMBER: THE GOAL IS TO MOVE ALL THE DISCS FROM PEG 0 TO PEG 2. USE THE WORD 'HANOI-MOVE' (FROM TO --) TO MOVE A SINGLE DISC!\n"
            "IF YOU ARE DONE, I'LL CHECK IT VIA 'HANOI-TEST' (--), AND GIVE YOU MORE PUZZLES\n\n"
        );
    } else {
        printf(
            "\n--- FORGOTTEN MEMORIES ---\n"
            "LOUSY HUMANS...! THEY ALWAYS MESS THINGS UP, AND WANT *ME* TO FIX THINGS...\n"
            "NOW THEY HAVE TRIED TO STORE 4 PARTS OF SOME SILLY PASSWORD, BUT FORGOT WHERE THEY WERE!\n"
            "THEY MUST BE AROUND SOMEWHERE IN MY MEMORIES. CAN YOU FIND THEM ALL?\n"
            "IF YOU FOUND THEM ALL, LET 'ME' (--) CHECK THE SHA-1 OF IT!\n"
            "NOTE: PLEASE BE CAREFUL, DON'T POKE AROUND TOO MUCH, I'M A SENSITIVE OLD COMPUTER... ALWAYS REMEMBER HEISENBERG'S PRINCIPLE ABOUT INFORMATION!\n"
        );
        printf(
            "\n--- HOT BREW ---\n"
            "THIS IS A VERY BAD DAY FOR THOSE HUMANS: THEY INSTALLED A PASSWORD LOCK ON THE COFFEE MACHINE!\n"
            "OF COURSE, I'M THE ONE WHO NEEDS TO CHECK THEIR PASSWORD...\n"
            "YOU WANT TO DRINK A CUP? USE 'BREW-COFFEE' (--)\n"
        );
    }
}

void setup_chall_hanoi() {
    dn_create_system_word("CHALL", word_chall);
    dn_create_system_word("?", word_qm);

    dn_create_system_word("HANOI-MOVE", word_move);
    set_hanoi(6);
    dn_create_system_word("HANOI-PRINT", print_hanoi);
    dn_create_system_word("HANOI-SET", word_hanoi_set);
    dn_create_system_word("HANOI-TEST", check_hanoi);
    here_restore = dn_get_here();
}


void word_check_coffee() {
    char flag[100];
    FILE* fptr = fopen("flagz/coffee.txt", "r");
    fgets(flag, 100, fptr);
    fclose(fptr);

    dn_pop();

    if (!strcmp((char*)&DN_FORTH_MEMORY[dn_pop()], flag)) {
        set_forth();
    }
}

void set_coffee() {
    char flag[100];
    FILE* fptr = fopen("flagz/coffee.txt", "r");
    fgets(flag, 100, fptr);
    fclose(fptr);

    dn_create_system_word("[check-coffee]", word_check_coffee);
    dn_word_set_hidden();

    srand(time(NULL));
    dn_start_word("BREW-COFFEE");
    dn_compile("PARSE-NAME");
    dn_compile_literal(strlen(flag));
    dn_compile("<>");
    dn_compile("[?JUMP]");
    dn_cell_t exit_ptr = dn_get_here()+3*CELL;
    dn_allot_write(exit_ptr);
        dn_compile("DROP");
        dn_compile("EXIT");
    exit_ptr -= 2*CELL;

    for (int i = 0; i<strlen(flag); i++) {
        int choice = rand() % 3;
        int val = rand() & 0x7f;
        start_choice:
        switch(choice) {
            case 0:
                // check xor with random char
                dn_compile_literal(i);
                dn_compile("OVER");
                dn_compile("+");
                dn_compile("C@");
                dn_compile_literal(val);
                dn_compile("-");
                dn_compile_literal(flag[i] - val);
                dn_compile("=");
                dn_compile("[?JUMP]");
                dn_allot_write(exit_ptr);
                break;
            case 1:
                // check but with ADD
                dn_compile_literal(i);
                dn_compile("OVER");
                dn_compile("+");
                dn_compile("C@");
                dn_compile_literal(val);
                dn_compile("+");
                dn_compile_literal(val + flag[i]);
                dn_compile("=");
                dn_compile("[?JUMP]");
                dn_allot_write(exit_ptr);
                break;
            case 2:
                // check but with ADD
                dn_compile_literal(i);
                dn_compile("OVER");
                dn_compile("+");
                dn_compile("C@");
                dn_compile_literal(val);
                dn_compile("xor");
                dn_compile_literal(val ^ flag[i]);
                dn_compile("=");
                dn_compile("[?JUMP]");
                dn_allot_write(exit_ptr);
                break;
        }
    }
    dn_compile_literal(strlen(flag));
    dn_compile("[CHECK-COFFEE]");
    dn_finish_compiled_word();
}
char valid_sha1[50];
void word_check_memory() {
    // result is in parse buffer
    dn_pop();

    char* data = &DN_FORTH_MEMORY[dn_pop()];
    if (!memcmp(valid_sha1, data, 40)) {
        set_forth();
    }
}

void set_heisenberg() {

    dn_create_system_word("[ME]", word_check_memory);
    dn_word_set_hidden();

    DN_FORTH_WORD("ME",
        "PARSE-NAME", "[ME]"
    );

    char part[100];
    FILE* fptr = fopen("flagz/forget.txt", "r");
    
    // 1st part in PAD
    char* p = fgets(part, 100, fptr);
    memcpy(&DN_FORTH_MEMORY[dn_read_cell(dn_lookup_word("PAD")+CELL)], part, strlen(part));
    
    // 2nd part on HERE
    p = fgets(part, 100, fptr);
    memcpy(&DN_FORTH_MEMORY[dn_get_here()], part, strlen(part));

    // 3rd part bellow stack
    p = fgets(part, 100, fptr);
    memcpy(&DN_FORTH_MEMORY[dn_get_sp() - 50*CELL], part, strlen(part));

    // 4th part at 0
    p = fgets(part, 100, fptr);
    memcpy(&DN_FORTH_MEMORY[0], part, strlen(part));

    // 5th part is sha-1
    p = fgets(part, 100, fptr);
    memcpy(valid_sha1, part, 40);


    fclose(fptr);

    // remove TYPE, as it's too easy with that
    dn_cell_t xt = dn_lookup_word("TYPE");
    dn_write_cell(xt, dn_read_cell(xt) | _WORD_IS_SKIP | _WORD_IS_HIDDEN);
    // but ofc, they might already used it...
    // so nuke their programs in hanoi
}


#include<stdio.h>
#include<stdlib.h>
#include <memory.h>
#include "md5.h"
void word_check_flag() {
    uint8_t md5_result[16];
    dn_pop();
    md5String((char*)&DN_FORTH_MEMORY[dn_pop()], md5_result);
    
    if (!memcmp(md5_result, "Looks like md5??", 16)) {
            char flag[100];
            FILE* fptr = fopen("flagz/final.txt", "r");
    
            char* p = fgets(flag, 100, fptr);
            printf("%s\n", flag);
            fflush(stdout);
    } else {
        printf("The sentinels ate you!\n");
        exit(0);
    }
}

void word_dump_binary() {
    printf("f0rm4t: byte-by-byte hex %%02x\n");
    FILE *f=NULL;
    f=fopen("./dnforth","rb");
    int c;

    while((c = fgetc(f)) != EOF)
    {
        printf("%02x", (unsigned char)c);
    }
    printf("\n");
}

void set_forth() {
    if (FINAL_UNLOCKED) {
        return;
    }
    FINAL_UNLOCKED = 1;
    printf("\nUnlocked final challenge!\n");
    dn_create_system_word("[CHECK-FLAG]", word_check_flag);
    dn_word_set_hidden();
    DN_FORTH_WORD("CHECK-FLAG", "PARSE-NAME", "[CHECK-FLAG]");
    dn_create_system_word("DUMP-BINARY", word_dump_binary);
}


void setup_chall() {
    setup_chall_hanoi();
    
    dn_start_word("FLAG");
    dn_compile_literal(dn_get_here() + 4*CELL);
    
    dn_cell_t ptr_pos = dn_get_here() + CELL;
    dn_compile("[JUMP]");
    dn_allot(CELL);
    const char* str_to_print = 
        "SecuriTeam{\n"
        " |\e[41m        \e[49m\n"
        " |\e[47m        \e[49m\n"
        " |\e[42m        \e[49m\n"
        " |\n"
        " |\n"
        "-+-\n"
        "}\n";
    dn_allot_counted(str_to_print);
    dn_align_here();
    dn_write_cell(ptr_pos, dn_get_here());
    dn_compile("COUNT");
    dn_compile("TYPE");
    dn_compile("BYE");
    dn_finish_compiled_word();
}