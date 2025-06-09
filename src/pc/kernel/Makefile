obj-m += chardev.o

chardev:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	#le linee di if servono a me perche uso clangd, evito di rimuovere i file di compilazione
	@if [ -f compile_commands.json ]; then cp compile_commands.json /tmp/compile_commands.json.backup; fi
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	@if [ -f /tmp/compile_commands.json.backup ]; then mv /tmp/compile_commands.json.backup compile_commands.json; fi
.PHONY: clean
