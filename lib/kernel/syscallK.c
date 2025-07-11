#include <kernel/syscall.h>
#include <stdint.h>
#include <kernel/thread.h>
#include <kernel/print.h>
#include <string.h>
#include <device/console.h>
#include <kernel/memory.h>


#define SYSCALL_NR      32


typedef void *syscall;
syscall syscallTable[SYSCALL_NR];


static uint32 sysGetpid() {
    return threadCurrent()->pid;
}


static uint32 sysWrite(char *str) {
    consolePuts(str);

    return strlen(str);
}


void syscallInit() {
    puts("syscall init starts.\n");

    syscallTable[SYS_GETPID] = sysGetpid;
    syscallTable[SYS_WRITE] = sysWrite;
    syscallTable[SYS_MALLOC] = sysMalloc;
    syscallTable[SYS_FREE] = sysFree;

    puts("syscall init done!\n");
}
