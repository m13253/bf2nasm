
CC = gcc
CFLAGS = -Wall -Werror -O3 -g3

.PHONY: all clean cleanall debug install uninstall

all: bf2nasm

clean:
	rm -f *.o awib.asm awib bf2nasm

debug: all
	./bf2nasm < awib.b > awib.asm
	nasm -o awib.o -f elf32 awib.asm
	ld -melf_i386 -o awib awib.o
	./awib < awib.b

install: all
	cp ./bf2nasm /usr/local/bin/bf2nasm

uninstall:
	rm -f /usr/local/bin/bf2nasm

