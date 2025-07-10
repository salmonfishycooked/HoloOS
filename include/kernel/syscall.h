#ifndef __INCLUDE_KERNEL_SYSCALL_H
#define __INCLUDE_KERNEL_SYSCALL_H

#include <stdint.h>

enum SYSCALL_FUNC {
    SYS_GETPID,
    SYS_WRITE
};

void syscallInit();

uint32 getpid();
uint32 write(char *str);

#endif
