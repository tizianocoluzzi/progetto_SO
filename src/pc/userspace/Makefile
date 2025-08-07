CC = gcc
flags = -Wall -g -std=c99 
DEPS = common.c
all: demone macro
%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)
macro: macro.o common.o
	$(CC) $(flags) -o macro macro.o common.o

demone: demone.o common.o
	$(CC) $(flags) -lpthread -o demone demone.o common.o
clean:
	rm ./demone.o
	rm ./common.o
	rm ./macro.o
	rm ./demone
	rm ./macro
.PHONY: clean
