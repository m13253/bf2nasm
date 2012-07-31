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

struct registers {
    unsigned long int eax;
    unsigned long int ebx;
    unsigned long int ecx;
    unsigned long int edx;
    unsigned long int esi;
    unsigned long int edi;
    unsigned char known;
}

/* registers.known */
#define RK_EAX 1
#define RK_EBX 2
#define RK_ECX 4
#define RK_EDX 8
#define RK_ESI 16
#define RK_EDI 32

/* Initialize the registers here.
   using 0xcc... rather than 0x00... is better for debugging */
struct registers registers = (
    /* eax */ 0xcccccccc,
    /* ebx */ 0xcccccccc,
    /* ecx */ 0xcccccccc,
    /* edx */ 0xcccccccc,
    /* esi */ 0xcccccccc,
    /* edi */ 0xcccccccc,
    /* known */ 0
);

int main(void)
{
    return 0;
}

/* vim: ts=4 sts=4 et
*/
