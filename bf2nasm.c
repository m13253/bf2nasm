/* bf2nasm - brainfuck to nasm compiler */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 *
 */

#include <stdio.h>
#include <stdlib.h>

/* Some consts */
#define BUFLEN 32

/* Types */
struct registers {
    unsigned long int eax;
    unsigned long int ebx;
    int ecx_is_edi;
    unsigned long int edx;
    int esi_is_edi;
    long int edioffset;
    signed char pedi;
    int pediabs;           /* is pedi absolute or relative */
    unsigned char known;   /* eax thru esi */
    unsigned char changed; /* all regs */
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
    /* ecx */ 0,
    /* edx */ 0xccccccccUL,
    /* esi */ 0,
    /* edi */ 0,
    /* pedi */ 0,
    /* pediabs */1,
    /* known */ RG_EDI | RG_PEDI,
    /* changed */ 0
};
char buffer[BUFLEN] = "";
size_t buffer_pointer = 0;
size_t buffer_length = 0;
size_t filepos = 0;
unsigned int loops[0x10000] = {0};
unsigned int nloop = 0;
unsigned int ploop = 0;

int fill_buffer(void);

int read_buffer(const size_t register offset)
{
    fill_buffer();
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
    fill_buffer();
    if(buffer_length)
    {
        register int tmp=buffer[buffer_pointer++];
        --buffer_length;
        while(buffer_pointer>=BUFLEN)
            buffer_pointer-=BUFLEN;
        ++filepos;
        return tmp;
    }
    else
        return EOF;
}

int dump_buffer(size_t count)
{
    fill_buffer();
    if(buffer_length>=count)
    {
        buffer_length-=count;
        buffer_pointer+=count;
        while(buffer_pointer>=BUFLEN)
            buffer_pointer-=BUFLEN;
        filepos+=count;
        return count;
    }
    else
    {
        register size_t len = buffer_length;
        buffer_pointer+=buffer_length;
        buffer_length=0;
        while(buffer_pointer>=BUFLEN)
            buffer_pointer-=BUFLEN;
        filepos+=len;
        return len;
    }
}

int fill_buffer(void)
{
    register size_t count=0;
    static int is_EOF=0;
    if(is_EOF)
        return count;
    while(buffer_length<BUFLEN)
    {
        register int tmp=getc(stdin);
        if(tmp==EOF)
        {
            is_EOF=1;
            return count;
        }
        if(tmp=='+' || tmp=='-' || tmp=='>' || tmp=='<' || tmp=='[' || tmp==']' || tmp==',' || tmp=='.')
        {
            push_buffer(tmp);
            ++count;
        }
    }
    return count;
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
    {\
        if(registers.eax)\
            printf("\tmov\teax, %lu\n", registers.eax);\
        else\
            printf("\txor\teax, eax\n");\
        registers.changed &= ~RG_EAX;\
    }\
}
#define push_ebx() {\
    if((registers.known & RG_EBX) && (registers.changed & RG_EBX))\
    {\
        if(registers.ebx)\
            printf("\tmov\tebx, %lu\n", registers.ebx);\
        else\
            printf("\txor\tebx, ebx\n");\
        registers.changed &= ~RG_EBX;\
    }\
}
#define push_edx() {\
    if((registers.known & RG_EDX) && (registers.changed & RG_EDX))\
    {\
        if(registers.edx)\
            printf("\tmov\tedx, %lu\n", registers.edx);\
        else\
            printf("\txor\tedx, edx\n");\
        registers.changed &= ~RG_EDX;\
    }\
}
#define push_esi() {\
    if((registers.known & RG_ESI) && (registers.changed & RG_ESI))\
        {printf("\tmov\tesi, %lu\n", registers.esi); registers.changed &= ~RG_ESI;}\
}
#define push_edi() {\
    if((registers.changed & RG_EDI) && registers.edioffset)\
    {\
        if(registers.edioffset>=0)\
            if(registers.edioffset==1)\
                printf("\tinc\tedi\n");\
            else\
                printf("\tadd\tedi, %ld\n", registers.edioffset);\
        else\
            if(registers.edioffset==-1)\
                printf("\tdec\tedi\n");\
            else\
                printf("\tsub\tedi, %ld\n", -registers.edioffset);\
        registers.changed &= ~RG_EDI; registers.edioffset=0;\
        registers.pediabs=0; registers.changed &= ~RG_PEDI; registers.pedi=0;\
    }\
}
#define push_pedi() {\
    if(registers.changed & RG_PEDI)\
        if(registers.pediabs)\
            {printf("\tmov\tbyte [edi], %u\n", (unsigned char) registers.pedi); registers.changed &= ~RG_PEDI;}\
        else if(registers.pedi)\
        {\
            if(registers.pedi>=0)\
                if(registers.pedi==1)\
                    printf("\tinc\tbyte [edi]\n");\
                else\
                    printf("\tadd\tbyte [edi], %d\n", registers.pedi);\
            else\
                if(registers.pedi==-1)\
                    printf("\tdec\tbyte [edi]\n");\
                else\
                    printf("\tsub\tbyte [edi], %d\n", -registers.pedi);\
            registers.changed &= ~RG_PEDI;\
            if(!registers.pediabs) registers.pedi=0;\
        }\
        else;\
    else;\
}

int match(const char *str, size_t offset)
{
    register char tmp;
    while((tmp=*(str++)))
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
            ++registers.pedi;
            registers.changed |= RG_PEDI;
            break;
        case '-':
            push_edi();
            --registers.pedi;
            registers.changed |= RG_PEDI;
            break;
        case '>':
            push_pedi();
            ++registers.edioffset;
            registers.pediabs = 0;
            registers.changed |= RG_EDI;
            registers.pedi = 0;
            registers.ecx_is_edi = 0;
            break;
        case '<':
            push_pedi();
            --registers.edioffset;
            registers.pediabs = 0;
            registers.changed |= RG_EDI;
            registers.pedi = 0;
            registers.ecx_is_edi = 0;
            break;
        case ',':
            if(!(registers.known & RG_EAX) || registers.eax!=3)
            {
                registers.eax=3;
                registers.known |= RG_EAX;
                registers.changed |= RG_EAX;
                push_eax();
            }
            if(!(registers.known & RG_EBX) || registers.ebx!=0)
            {
                registers.ebx=0;
                registers.known |= RG_EBX;
                registers.changed |= RG_EBX;
                push_ebx();
            }
            registers.known &= ~RG_ECX;
            registers.changed &= ~RG_ECX;
            push_edi();
            if(!registers.ecx_is_edi)
            {
                fputs("\tmov\tecx, edi\n", stdout);
                registers.ecx_is_edi=1;
            }
            if(!(registers.known & RG_EDX) || registers.edx!=1)
            {
                registers.edx=1;
                registers.known |= RG_EDX;
                registers.changed |= RG_EDX;
                push_edx();
            }
            fputs("\tint\t80h\n", stdout);
            registers.known &= ~RG_EAX;
            registers.changed &= ~RG_EAX;

            registers.pediabs = 0;
            registers.pedi = 0;
            registers.changed &= ~RG_PEDI;
            break;
        case '.':
            if(!(registers.known & RG_EAX) || registers.eax!=4)
            {
                registers.eax=4;
                registers.known |= RG_EAX;
                registers.changed |= RG_EAX;
                push_eax();
            }
            if(!(registers.known & RG_EBX) || registers.ebx!=1)
            {
                registers.ebx=1;
                registers.known |= RG_EBX;
                registers.changed |= RG_EBX;
                push_ebx();
            }
            registers.known &= ~RG_ECX;
            registers.changed &= ~RG_ECX;
            push_edi();
            push_pedi();
            if(!registers.ecx_is_edi)
            {
                fputs("\tmov\tecx, edi\n", stdout);
                registers.ecx_is_edi=1;
            }
            if(!(registers.known & RG_EDX) || registers.edx!=1)
            {
                registers.edx=1;
                registers.known |= RG_EDX;
                registers.changed |= RG_EDX;
                push_edx();
            }
            fputs("\tint\t80h\n", stdout);
            registers.known &= ~RG_EAX;
            registers.known &= ~RG_EAX;
            registers.changed &= ~RG_EAX;
            break;
        case '[':
            if((registers.known & RG_PEDI) && registers.pediabs && !registers.pedi)
            {
                register unsigned int bralevel = 1;
                do
                {
                    tmp=pop_buffer();
                    if(tmp=='[') ++bralevel;
                    else if(tmp==']') --bralevel;
                }
                while(bralevel && tmp!=EOF);
            }
            else if(read_buffer(0)==']')
                if((registers.known & RG_PEDI) && registers.pediabs && registers.pedi)
                {
                    ++nloop;
                    printf("b%u:\n\tjmp\tb%u\n", nloop, nloop);
                    do
                        tmp=pop_buffer();
                    while(tmp!=EOF);
                }
                else
                {
                    push_edi();
                    push_pedi();
                    ++nloop;
                    printf("\tcmp\tbyte [edi], 0\n\tje\te%u\nb%u:\n\tjmp\tb%u\ne%u:\n", nloop, nloop, nloop, nloop);
                    registers.changed |= RG_PEDI;
                    registers.pediabs = 1;
                    registers.pedi = 0;
                    pop_buffer();
                }
            else if(match("-]", 0) || match("+]", 0))
            {
                registers.known |= RG_PEDI;
                registers.changed |= RG_PEDI;
                registers.pediabs = 1;
                registers.pedi = 0;
                dump_buffer(2);
            }
            else if(match("->+<]", 0))
            {
                fputs("\tmov\tal, byte [edi]\n\tadd\tbyte [edi+1], al\n\txor\teax, eax\n\tmov\tbyte [edi], al\n", stdout);
                registers.known |= RG_EAX;
                registers.changed &= ~RG_EAX;
                registers.eax = 0;
                dump_buffer(5);
            }
            else if(match("->+>+<<]", 0))
            {
                fputs("\tmov\tal, byte [edi]\n\tadd\tbyte [edi+1], al\n\tadd\tbyte [edi+2], al\n\txor\teax, eax\n\tmov\tbyte [edi], al\n", stdout);
                registers.known |= RG_EAX;
                registers.changed &= ~RG_EAX;
                registers.eax = 0;
                dump_buffer(8);
            }
            else
            {
                push_edi();
                push_pedi();
                loops[ploop]=nloop++;
                printf("b%u:\t\t\t\t; Level %u {\n\tcmp\tbyte [edi], 0\n\tje\te%u\n", loops[ploop], ploop+1, loops[ploop]);
                registers.changed &= ~RG_PEDI;
                registers.pediabs = 0;
                registers.pedi = 0;
                ++ploop;
            }
            break;
        case ']':
            if(ploop<=0)
            {
                fprintf(stderr, "Fatal: %zu: Operation expected but got ‘]’.\n", filepos);
                exit(1);
            }
            --ploop;
            push_edi();
            push_pedi();
            printf("\tjmp\tb%u\ne%u:\t\t\t\t; Level %u }\n", loops[ploop], loops[ploop], ploop+1);
            registers.known |= RG_PEDI;
            registers.changed &= ~RG_PEDI;
            registers.pediabs = 0;
            registers.pedi = 0;
            break;
    }
}

void finish(void)
{
    if(ploop!=0)
    {
        fprintf(stderr, "Fatal: %zu: ‘]’ expected but got EOF.\n", filepos);
        exit(1);
    }
    if(!(registers.known & RG_EAX) || registers.eax!=1)
    {
        registers.eax=1;
        registers.known |= RG_EAX;
        registers.changed |= RG_EAX;
        push_eax();
    }
    if(!(registers.known & RG_EBX) || registers.ebx!=0)
    {
        registers.ebx=0;
        registers.known |= RG_EBX;
        registers.changed |= RG_EBX;
        push_ebx();
    }
    fputs("\tint\t80h\n", stdout);
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

    while(buffer_length || fill_buffer())
        process();

    finish();

    return 0;
}

/* vim: ts=4 sts=4 et
*/
