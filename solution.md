# 100 sides of a square

The 4 challenges are ordered in a diamond pattern: Hot brew and forgotten memories are locked by Shall we play a game, and Breaking the 100th wall is locked until either one of them is solved.

The word `CHALL` is provided to show what challenges are alaviable and give a short description on them.

## Shall we play a game

The task is to implement a classic tower-of-hanoi program in FORTH. Since most players will be unfamiliar with this language, I chose this simple exercise.

They must use the given word to move a disc, and the solution will be checked in the background.

The program is chaked from 3 to 8 discs, and must pass all of them.

A possible solution, based on https://www.strobotics.com/hanoi.htm:
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
```

Solving this unlocks 2 challenges

## Hot brew

This is a simple keygenme, but as I was lazy and generating the code on the fly, it might throw people a bit off, as they will get a different check function each time

They must crack the program `brew-coffee`, that simply check each character individually, by applying XOR, - or + to them and comparing to a constant.

This challenge is impossible without using DISAS, and is intended as a familiarization challange for that tool and decompiling FORTH code with it.

## Forgotten memories

This is a simple forensics: 4 parts of the flag are scattered around the memory. Simply dump it and you got it... except a little twist
- the "TYPE" (~print) word mysteriously vanishes
- one of the pieces sits **right** at a transient area, so any number converion (from `.`, for example), or new word definition destroys it (hence the hint about measurement)

The pieces are at
- transient area used for words
- 50 cells bellow stack pointer
- 0
- in PAD

If you assemble them (they are numbered), you can use the word `me` to check their SHA-1 hash (and unlock the next level)

## Breaking the 100th wall

2 new words are installed:
- dump-binary
- check-flag

Dump-binary does what it says, and even prints the format.
Check-flag does an if(true) print(flag) in C.
(the if(true) part is via an impossible MD5 - I hope they realize that the result looks suspiciously like TEXT)

The solution is simple: the VM has C pointers for words defined in C, so by moving that pointer by a tiny bit, we can skip the check. also the program crashes, as the stack is messed up, but a fflush() before the return guarantees that the flag will reach the user

