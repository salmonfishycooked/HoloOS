#ifndef __INCLUDE_KERNEL_SYSCALL_H
#define __INCLUDE_KERNEL_SYSCALL_H

#include <stdint.h>

enum SYSCALL_FUNC {
    SYS_GETPID,
    SYS_WRITE,
    SYS_MALLOC,
    SYS_FREE
};

void syscallInit();

uint32 getpid();
uint32 write(char *str);
void *malloc(uint32 size);
void free(void *ptr);

#endif
