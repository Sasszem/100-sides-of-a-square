CFLAGS = -g -O2

OBJS = forth.o main.o words/arith.o words/core.o words/memory_stack.o words/forth_words.o words/control.o words/io.o words/aux.o words/words.o words.o compile.o debug.o execute.o memory.o md5.o chall.o

dnforth: $(OBJS)
	$(CC) -o dnforth $(OBJS)
clean:
	rm -f dnforth *.o words/*.o
