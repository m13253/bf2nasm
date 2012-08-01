# bf2nasm
Brainfuck to nasm compiler

## Have a try!
Just clone this repo, and do what you want!

	$ git clone https://github.com/m13253/bf2nasm.git

### Compile

	$ make

### Compile and run awib to compile awib.b itself

	$ make debug

### Clean up useless files
Remove all the intermediate files (e.g. `*.o`, `test.asm` and `test`)
and `bf2nasm`.

	$ make clean

### Compile your own brainfuck program:
`bf2nasm` takes stdin as its input, and output the compiled nasm source
code to stdout.

	$ # make sure you have already compiled bf2nasm, if not, type:
	$ #		make
	$ # And you can clean up the intermediate files by typing:
	$ #		make clean
	$ ./bf2nasm < input.b > output.asm

Then you will get a well formatted nasm source code. To compile it
into an executable file, type:

	$ nasm -f elf32 -o output.o output.asm  # compile
	$ ld -melf_i386 -o output output.o      # link

#### Everything in one script:

	BFIN="input.b"
	BFOUT="output"
	make
	make clean
	./bf2nasm < "$BFIN" > "$BFOUT.asm"
	nasm -f elf32 -o "$BFOUT.o" "$BFOUT.asm"
	ld -melf_i386 -o "$BFOUT" "$BFOUT.o"

## License
Licensed under LGPLv3.<br>
*ABSOLUTELY NO WARRANTY!*

