#include <kernel/syscall.h>
#include <stdint.h>
#include <kernel/thread.h>
#include <kernel/print.h>

#define SYSCALL_NR      32

typedef void *syscall;
syscall syscallTable[SYSCALL_NR];

static uint32 sysGetpid() {
    return threadCurrent()->pid;
}

void syscallInit() {
    puts("syscall init starts.\n");

    syscallTable[SYS_GETPID] = sysGetpid;

    puts("syscall init done!\n");
}
