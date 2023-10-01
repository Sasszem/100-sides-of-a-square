# 100 sides of a square

The 4 challenges are ordered in a diamond pattern: Hot brew and forgotten memories are locked by Shall we play a game, and Breaking the 100th wall is locked until either one of them is solved.

The word `CHALL` is provided to show what challenges are alaviable and give a short description on them.

## Shall we play a game

`CHALL` reports:
> --- SHALL WE PLAY A GAME? ---   
> SHALL WE PLAY A GAME? WHAT ABOUT 'GLOBAL THERMONUCLEAR WAR'? WHAT? WHY NOT?   
> YOU DO NOT LIKE WAR GAMES? WHAT ABOUT A PUZZLE THEN? 'TOWERS OF HANOI' MAYBE?   
> WHAT? YOU WANT ME TO SOLVE IT? NO, THAT WOULD BE TOO EASY!   
> IT WOULD BE WAY MORE INTERESTING IF *YOU* SOLVED IT!   
> WHAT NOW? YOU DO NOT LIKE BORING STUFF? HUMANS... OKAY, NEW PLAN: YOU WRITE A PROGRAM TO SOLVE THE PUZZLE, OK?   
> LET'S SEE... CREATE A WORD NAMED 'HANOI' (N -- ) THAT SOLVES THE PUZZLE FOR N DISCS!   
> I ALREADY HAVE A PROGRAM TO SETUP THE PUZZLE NAME 'HANOI-SET' (N--), AND EVEN A PRINT PROGRAM 'HANOI-PRINT' (--)!   
> REMEMBER: THE GOAL IS TO MOVE ALL THE DISCS FROM PEG 0 TO PEG 2. USE THE WORD 'HANOI-MOVE' (FROM TO --) TO MOVE A SINGLE DISC!   
> IF YOU ARE DONE, I'LL CHECK IT VIA 'HANOI-TEST' (--), AND GIVE YOU MORE PUZZLES   


The task is to implement a classic tower-of-hanoi program in FORTH. Since most players will be unfamiliar with this language, I chose this simple exercise.

They must use the given word to move a disc, and the solution will be checked in the background.

The program is chaked from 3 to 8 discs, and must pass all of them.

A possible solution, based on https://www.strobotics.com/hanoi.htm:
```forth
VARIABLE SA
VARIABLE SB
VARIABLE SC
VARIABLE N
: HANOI-INT
 SC ! SB ! SA ! N !
 N @ 0= IF EXIT THEN
 N @ SA @ SB @ SC @
 N @ 1 - SA @ SC @ SB @ RECURSE
 SC ! SB ! SA ! N !
 sa @ 1- sb @ 1- hanoi-move
 N @ 1 - SC @ SB @ SA @ RECURSE
;

: hanoi 1 3 2 hanoi-int ;
```

After this, `hanoi-test` can be run and gives us a flag.
The correct flag can be found in `flagz/hanoi.txt`.

Solving this unlocks 2 challenges

## Hot brew

`CHALL` reports:

> --- MORNING ROUTINE ---  
> THIS IS A VERY BAD DAY: THEY INSTALLED A PASSWORD LOCK ON THE COFFEE MACHINE!  
> CAN YOU GET ME SOME COFFEE?  
> BREW-COFFEE (--)

This is a simple keygenme, but as I was lazy and generating the code on the fly, it might throw people a bit off, as they will get a different check function each time

They must crack the program `brew-coffee`, that simply check each character individually, by applying XOR, - or + to them and comparing to a constant.

This challenge is impossible without using DISAS, and is intended as a familiarization challange for that tool and decompiling FORTH code with it.

The correct flag can be found in `flagz/coffee.txt`.

## Forgotten memories

> --- FORGOTTEN MEMORIES ---   
> LOUSY HUMANS...! THEY ALWAYS MESS THINGS UP, AND WANT *ME* TO FIX THINGS...   
> NOW THEY HAVE TRIED TO STORE 4 PARTS OF SOME SILLY PASSWORD, BUT FORGOT WHERE THEY WERE!   
> THEY MUST BE AROUND SOMEWHERE IN MY MEMORIES. CAN YOU FIND THEM ALL?   
> IF YOU FOUND THEM ALL, LET 'ME' (--) CHECK THE SHA-1 OF IT!   
> NOTE: PLEASE BE CAREFUL, DON'T POKE AROUND TOO MUCH, I'M A SENSITIVE OLD COMPUTER... ALWAYS REMEMBER HEISENBERG'S PRINCIPLE ABOUT INFORMATION!   

This is a simple forensics: 4 parts of the flag are scattered around the memory. Simply dump it and you got it... except a little twist
- the "TYPE" (~print, would make dumping memory easier) word *mysteriously* vanishes (along with any programs defined by the user, so they won't easily get around it)
- one of the pieces sits **right** at a transient area, so any number converion (from `.`, for example), or new word definition destroys it (hence the hint about measurement)

The pieces are at
1. in PAD
2. transient area used for words
3. 50 cells bellow stack pointer
4. starting at 0 (technicly ret stack, but hadly ever reaches there)


If you assemble them (they are numbered), you can use the word `me` to check their SHA-1 hash (and unlock the next level)

(the parts are loaded from file `flagz/forget.txt`, the last line is the SHA-1)

Note: For this chall, I decreased the global used memory size from 1<<18 bytes to 1<<16 in the entire program. The remaining memory is still usable, but I set the SP to start at 1<<16, and nothing uses memory above that

Possible solution:
```
40 cells allot ( move PTR so "fix" the transient area )
: type 0 ?do dup i + c@ emit loop drop ;
( this is basically the implementation of TYPE )

pad 40 type ( part 1)
here 60 cells - 60 cells type ( part 2 )
sp@ -100 cells + dup 100 cells type ( part 3)
0 40 type ( part 4)
```

## Breaking the 100th wall

> --- 4th wall bReaK ---    
> HEY, DON'T YOU POKE AROUND IN ME TOO MUCH, IT TICKLES!    
> OH, YOU WANT TO KNOW WHAT MAKES ME TICK?    
> DO YOU REALLY THINK YOU CAN HANDLE THE TRUTH?    
> fine. you never saw my true self. i'll show you, but there is no going back to the lies    
> do you want to know how deep the rabbit-hole goes?    
> can you see through the cracks in the 100th wall?    
> - DUMP-BINARY (--)    
> - CHECK-FLAG (--)    


2 new words are installed:
- dump-binary
- check-flag

Dump-binary does what it says, and even prints the format.
(byte-by-byte, %02x format, can be decoded in various ways)

Check-flag does an if(false) print(flag) in C.
(the if(false) part is via an impossible MD5 - I hope they realize that the result looks suspiciously like TEXT)

The solution is simple: the VM has C pointers for words defined in C, so by moving that pointer by a tiny bit, we can skip the check. also the program crashes, as the stack is messed up, but a fflush() before the return guarantees that the flag will reach the user

Here is the code that sets up the final challenge:
```c
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
```

And here's the `word_check_flag` that implements `[CHECK-FLAG]`:

```c
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
```

And here's the disassembly, from GDB (intel syntax):
(note: recompilation might change this, but will look similar to this)

```
   0x0000000000009980 <+0>:	    endbr64 
   0x0000000000009984 <+4>:	    push   rbx
   0x0000000000009985 <+5>:	    add    rsp,0xffffffffffffff80
   0x0000000000009989 <+9>:	    mov    rax,QWORD PTR fs:0x28
   0x0000000000009992 <+18>:	mov    QWORD PTR [rsp+0x78],rax
   0x0000000000009997 <+23>:	xor    eax,eax
   0x0000000000009999 <+25>:	call   0x8bf0 <dn_pop>
   0x000000000000999e <+30>:	xor    eax,eax
   0x00000000000099a0 <+32>:	call   0x8bf0 <dn_pop>
   0x00000000000099a5 <+37>:	mov    rsi,rsp
   0x00000000000099a8 <+40>:	mov    edi,eax
   0x00000000000099aa <+42>:	lea    rax,[rip+0x66cf]        # 0x10080 <DN_FORTH_MEMORY>
   0x00000000000099b1 <+49>:	add    rdi,rax
   0x00000000000099b4 <+52>:	call   0x96c0 <md5String>
   0x00000000000099b9 <+57>:	movabs rax,0x3f3f35646d20656b
   0x00000000000099c3 <+67>:	xor    rax,QWORD PTR [rsp+0x8]
   0x00000000000099c8 <+72>:	movabs rdx,0x696c20736b6f6f4c
   0x00000000000099d2 <+82>:	xor    rdx,QWORD PTR [rsp]
   0x00000000000099d6 <+86>:	or     rax,rdx
   0x00000000000099d9 <+89>:	jne    0x9a30 <word_check_flag+176>
   0x00000000000099db <+91>:	lea    rsi,[rip+0x209e]        # 0xba80
   0x00000000000099e2 <+98>:	lea    rdi,[rip+0x1fbc]        # 0xb9a5
   0x00000000000099e9 <+105>:	call   0x1300 <fopen@plt>
   0x00000000000099ee <+110>:	lea    rbx,[rsp+0x10]
   0x00000000000099f3 <+115>:	mov    esi,0x64
   0x00000000000099f8 <+120>:	mov    rdx,rax
   0x00000000000099fb <+123>:	mov    rdi,rbx
   0x00000000000099fe <+126>:	call   0x1280 <fgets@plt>
   0x0000000000009a03 <+131>:	mov    rdi,rbx
   0x0000000000009a06 <+134>:	call   0x11e0 <puts@plt>
   0x0000000000009a0b <+139>:	mov    rdi,QWORD PTR [rip+0x664e]        # 0x10060 <stdout@GLIBC_2.2.5>
   0x0000000000009a12 <+146>:	call   0x12e0 <fflush@plt>
   0x0000000000009a17 <+151>:	mov    rax,QWORD PTR [rsp+0x78]
   0x0000000000009a1c <+156>:	sub    rax,QWORD PTR fs:0x28
   0x0000000000009a25 <+165>:	jne    0x9a43 <word_check_flag+195>
   0x0000000000009a27 <+167>:	sub    rsp,0xffffffffffffff80
   0x0000000000009a2b <+171>:	pop    rbx
   0x0000000000009a2c <+172>:	ret    
   0x0000000000009a2d <+173>:	nop    DWORD PTR [rax]
   0x0000000000009a30 <+176>:	lea    rdi,[rip+0x1f7e]        # 0xb9b5
   0x0000000000009a37 <+183>:	call   0x11e0 <puts@plt>
   0x0000000000009a3c <+188>:	xor    edi,edi
   0x0000000000009a3e <+190>:	call   0x1310 <exit@plt>
   0x0000000000009a43 <+195>:	call   0x1220 <__stack_chk_fail@plt>
```

As you can see, the "valid" MD5 hash is inlined. That might throw a few people off, who would try to break it. 

The check happens at <+89>: `jne    0x9a30 <word_check_flag+176>`. The loading & printing of the flag happens from <+91> to <+146>, followed by a stack canary check.

If we make the system jump to <+91> instead of the entry, it will print the flag. It will also crash the program, but `fflush` makes sure that we get the flag out of it.

To make that happen, we must understand how are primitive words implemented in DN-FORTH.

Here's the output of `disas emit`:
```
#########################################
# Disassembling word at 0081ch - 'EMIT' #
#########################################

HEADER:
0081c	841007f8
00820	0000558b
00824	59fbd6e0

Link field ptr: 007f8 ('1-')
Name len: 4

Attributes:
- PRIMITIVE (0x558b59fbd6e0)


NAME AREA:
00828	454d4954	EMIT
```

The disassembler provides loads of valuable info - the most important is the attributes area, where we can see a valid memory address of the function implementing this word.

You can also see that it's stored as PFA:CFA

So all we have to do is to modify the word `[CHECK-FLAG]` and increment the pointer.

```forth
' [check-flag]  ( pushes XT - address of word )
2 cells +       ( cfa address )
dup @ 91 +      ( read addr & increment )
swap !          ( write back )
[check-flag]    ( execute... )
```

# SPEED-unlock for testing

```

VARIABLE SA
VARIABLE SB
VARIABLE SC
VARIABLE N
: HANOI-INT
 SC ! SB ! SA ! N !
 N @ 0= IF EXIT THEN
 N @ SA @ SB @ SC @
 N @ 1 - SA @ SC @ SB @ RECURSE
 SC ! SB ! SA ! N !
 sa @ 1- sb @ 1- hanoi-move
 N @ 1 - SC @ SB @ SA @ RECURSE
;

: hanoi 1 3 2 hanoi-int ;

hanoi-test

( copy til here to unlock 2nd level )

brew-coffee SecuriTeam{MD5(418_im_4_t34p0t_n0_c0ff33_f0r_y0u)}

```