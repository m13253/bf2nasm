
CC = gcc
NASM = nasm
LD = ld
RM = rm
CFLAGS = -Wall -Werror -O3 -g3

.PHONY: all clean cleanall debug install uninstall

all: bf2nasm

clean:
	$(RM) -f *.o *.asm awib bf2nasm

%.asm: %.b bf2nasm
	./bf2nasm < $< > $@

%.o: %.asm
	$(NASM) -f elf32 -o $@ $<

%: %.o
	$(LD) -m elf_i386 -o $@ $<

install: all
	cp ./bf2nasm /usr/local/bin/bf2nasm

uninstall:
	rm -f /usr/local/bin/bf2nasm

