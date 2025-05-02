CC = gcc
flags = -Wall -g std=c99 

demone:
	$(CC) $(flags) -o demone demone.c

obj-m += chardev.o

chardev:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rm ./demone

.PHONY: clean
