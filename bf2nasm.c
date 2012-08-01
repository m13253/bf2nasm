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
    long int pedi;
    int pediabs;
    unsigned char known;
    unsigned char changed;
};

/* registers.known */
#define RG_EAX 1
#define RG_EBX 2
#define RG_ECX 4
#define RG_EDX 8
#define RG_ESI 16
#define RG_EDI 32
#define RG_PEDI 64

/* Initialize the registers here.
   using 0xcc... rather than 0x00... is better for debugging */
struct registers registers = {
    /* eax */ 0xccccccccUL,
    /* ebx */ 0xccccccccUL,
    /* ecx */ 0xccccccccUL,
    /* edx */ 0xccccccccUL,
    /* esi */ 0xccccccccUL,
    /* edi */ 0,
    /* pedi */ 0,
    /* pediabs */1,
    /* known */ 0
    /* changed */ 0
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

int fill_buffer(void)
{
    while(buffer_length<BUFLEN)
    {
        register int tmp=getc(stdin);
        if(tmp==EOF)
            return tmp;
        if(tmp=='+' || tmp=='-' || tmp=='>' || tmp=='<' || tmp=='[' || tmp==']' || tmp==',' || tmp=='.')
            push_buffer(tmp);
        print_buffer();
    }
}

void print_buffer(void)
{
    register int _i;
    fprintf(stderr, "pointer=%zu, length=%zu, ", buffer_pointer, buffer_length);
    for(_i=0; _i<BUFLEN; ++_i)
        putc(buffer[_i], stderr);
    fputc('\n', stderr);
}

#define push_eax() {\
    if((registers.known & RG_EAX) && (registers.changed & RG_EAX))\
        {printf("\tmov\teax, %lu\n", registers.eax); registers.changed &= ~RG_EAX;}\
}
#define push_ebx() {\
    if((registers.known & RG_EBX) && (registers.changed & RG_EBX))\
        {printf("\tmov\tebx, %lu\n", registers.ebx); registers.changed &= ~RG_EBX;}\
}
#define push_ecx() {\
    if((registers.known & RG_ECX) && (registers.changed & RG_ECX))\
        {printf("\tmov\tecx, %lu\n", registers.ecx); registers.changed &= ~RG_ECX;}\
}
#define push_edx() {\
    if((registers.known & RG_EDX) && (registers.changed & RG_EDX))\
        {printf("\tmov\tedx, %lu\n", registers.edx); registers.changed &= ~RG_EDX;}\
}
#define push_esi() {\
    if((registers.known & RG_ESI) && (registers.changed & RG_ESI))\
        {printf("\tmov\tesi, %lu\n", registers.esi); registers.changed &= ~RG_ESI;}\
}
#define push_edi() {\
    if((registers.known & RG_EDI) && (registers.changed & RG_EDI) && registers.edi)\
    {\
        printf("\t%s\tedi, %lu\n", registers.edx>=0 ? "add" : "sub", registers.edi);\
        registers.changed &= ~RG_EDI; registers.edi=0;\
    }\
}
#define push_pedi() {\
    if((registers.known & RG_PEDI) && (registers.changed & RG_PEDI))\
        if(registers.pediabs)\
            {printf("\tmov\tbyte [edi], %lu\n", registers.edi); registers.changed &= ~RG_PEDI;}\
        else if(registers.pedi)\
        {\
            printf("\t%s\tbyte [edi], %lu\n", registers.edx>=0 ? "add" : "sub", registers.pedi);\
            registers.changed &= ~RG_PEDI; registers.edi=0;\
        }\
}

int match(const char *str, size_t offset)
{
    register char tmp;
    while(tmp=*(str++))
        if(read_buffer(offset++)!=tmp)
            return 0;
    return 1;
}

void process(void)
{
    register int tmp;
    tmp=pop_buffer();
    switch(tmp)
    {
        case '+':
            push_edi();
            ++registers.pedioffset;
            break;
        case '-':
            push_edi();
            --registers.pedioffset;
            break;
        case '>':
            push_pedi();
            ++registers.edioffset;
            break;
        case '<':
            push_pedi();
            --registers.edioffset;
            break;
    }
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

    while(fill_buffer()!=EOF)
    {
        if(buffer_length>=BUFLEN)
            process();
    }
    if(buffer_length)
        process();

    return 0;
}

/* vim: ts=4 sts=4 et
*/
