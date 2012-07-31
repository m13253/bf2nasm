/* bf2nasm - brainfuck to nasm compiler */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 */

#include <stdio.h>

/* Some consts */
#define BUFLEN 32

/* Types */
struct registers {
    unsigned long int eax;
    unsigned long int ebx;
    unsigned long int ecx;
    unsigned long int edx;
    unsigned long int esi;
    long int edioffset;
    unsigned char known;
};

/* registers.known */
#define RK_EAX 1
#define RK_EBX 2
#define RK_ECX 4
#define RK_EDX 8
#define RK_ESI 16

/* Initialize the registers here.
   using 0xcc... rather than 0x00... is better for debugging */
struct registers registers = {
    /* eax */ 0xccccccccUL,
    /* ebx */ 0xccccccccUL,
    /* ecx */ 0xccccccccUL,
    /* edx */ 0xccccccccUL,
    /* esi */ 0xccccccccUL,
    /* edi */ 0,
    /* known */ 0
};
char buffer[BUFLEN] = "";
size_t buffer_pointer = 0;
size_t buffer_length = 0;

int read_buffer(const size_t register offset)
{
    if(buffer_length>offset)
        if(buffer_pointer+offset<BUFLEN)
            return buffer[buffer_pointer+offset];
        else
            return buffer[buffer_pointer+offset-BUFLEN];
    else
        return EOF;
}

int push_buffer(const char ch)
{
    if(buffer_length>=BUFLEN)
        return EOF;
    if(buffer_pointer+buffer_length<BUFLEN)
        buffer[buffer_pointer+(buffer_length++)]=ch;
    else
        buffer[buffer_pointer+(buffer_length++)-BUFLEN]=ch;
    return ch;
}

int pop_buffer(void)
{
    if(buffer_length)
    {
        --buffer_length;
        return buffer[buffer_pointer++];
    }
    else
        return EOF;
}

void print_buffer(void)
{
    register int _i;
    fprintf(stderr, "pointer=%zu, length=%zu, ", buffer_pointer, buffer_length);
    for(_i=0; _i<BUFLEN; ++_i)
        putc(buffer[_i], stderr);
    fputc('\n', stderr);
}

void process(void)
{
    register int tmp;
    tmp=pop_buffer();
}

int main(void)
{
    fputs(
        "; Generated by bf2nasm, https://github.com/m13253/bf2nasm\n"
        "[global _start]\n"
        "\n"
        "[section .bss]\n"
        "mem:\tresb 300000\n"
        "\n"
        "[section .text]\n"
        "\n"
        "_start:\n"
        "\tmov\tedi, mem\n",
        stdout);

    do
    {
        register int tmp=getc(stdin);
        if(tmp=='+' || tmp=='-' || tmp=='>' || tmp=='<' || tmp=='[' || tmp==']' || tmp==',' || tmp=='.')
            push_buffer(tmp);
        print_buffer();
        if(buffer_length>=BUFLEN)
            process();
    }
    while(tmp!=EOF);
    if(buffer_length)
        process();

    return 0;
}

/* vim: ts=4 sts=4 et
*/
