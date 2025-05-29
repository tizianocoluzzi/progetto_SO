CC = gcc
flags = -Wall -g -std=c99 
DEPS = common.c
all: demone chardev macro
%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)
macro: macro.o common.o
	$(CC) $(flags) -o macro macro.o common.o

demone: demone.o common.o
	$(CC) $(flags) -o demone demone.o common.o

obj-m += chardev.o

chardev:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	@if [ -f compile_commands.json ]; then cp compile_commands.json /tmp/compile_commands.json.backup; fi
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rm ./demone
	@if [ -f /tmp/compile_commands.json.backup ]; then mv /tmp/compile_commands.json.backup compile_commands.json; fi
.PHONY: clean
