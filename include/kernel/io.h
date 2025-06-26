/**
 * io.h defines a set of operations related to I/O port
 */


#ifndef __INCLUDE_KERNEL_IO_H
#define __INCLUDE_KERNEL_IO_H

#include "stdint.h"

// outb writes 1 byte to port
static inline void outb(uint16 port, uint8 data) {
    asm volatile ("outb %b0, %w1" : : "a"(data), "Nd"(port));
}

// outsw writes wordCnt of words started with addr to port
static inline void outsw(uint16 port, const void *addr, uint32 wordCnt) {
    asm volatile ("cld; rep outsw;" : "+S"(addr), "+c"(wordCnt) : "d"(port));
}

// inb reads 1 byte from port
static inline uint8 inb(uint16 port) {
    uint8 data;
    asm volatile ("inb %w1, %b0" : "=a"(data) : "Nd"(port));

    return data;
}

// insw reads wordCnt of words from port to address started with addr
static inline void insw(uint16 port, void *addr, uint32 wordCnt) {
    asm volatile ("cld; rep insw" : "+D"(addr), "+c"(wordCnt) : "d"(port) : "memory");
}

#endif
