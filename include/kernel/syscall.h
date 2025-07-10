#ifndef __INCLUDE_KERNEL_SYSCALL_H
#define __INCLUDE_KERNEL_SYSCALL_H

#include <stdint.h>

enum SYSCALL_FUNC {
    SYS_GETPID
};

void syscallInit();

uint32 getpid();

#endif
