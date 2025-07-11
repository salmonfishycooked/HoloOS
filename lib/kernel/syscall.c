#include <kernel/syscall.h>
#include <kernel/print.h>

#define _syscall0(NUMBER) ({                    \
    int retval;                                 \
    asm volatile ("int $0x80"                   \
                  : "=a"(retval)                \
                  : "a"(NUMBER)                 \
                  : "memory"                    \
    );                                          \
    retval;                                     \
})

#define _syscall1(NUMBER, ARG1) ({              \
    int retval;                                 \
    asm volatile ("int $0x80"                   \
                  : "=a"(retval)                \
                  : "a"(NUMBER), "b"(ARG1)      \
                  : "memory"                    \
    );                                          \
    retval;                                     \
})

#define _syscall2(NUMBER, ARG1, ARG2) ({                   \
    int retval;                                            \
    asm volatile ("int $0x80"                              \
                  : "=a"(retval)                           \
                  : "a"(NUMBER), "b"(ARG1), "c"(ARG2)      \
                  : "memory"                               \
    );                                                     \
    retval;                                                \
})

#define _syscall3(NUMBER, ARG1, ARG2, ARG3) ({                        \
    int retval;                                                       \
    asm volatile ("int $0x80"                                         \
                  : "=a"(retval)                                      \
                  : "a"(NUMBER), "b"(ARG1), "c"(ARG2), "d"(ARG3)      \
                  : "memory"                                          \
    );                                                                \
    retval;                                                           \
})


/**
 * below are user interfaces for calling real syscall function in kernel.
 */


// getpid used for getting current id of process.
uint32 getpid() {
    return _syscall0(SYS_GETPID);
}


// write writes str onto screen.
uint32 write(char *str) {
    return _syscall1(SYS_WRITE, str);
}


// malloc allocates memory of size bytes, and returns its start address.
void *malloc(uint32 size) {
    return (void *) _syscall1(SYS_MALLOC, size);
}


// free reclaims memory started at ptr.
void free(void *ptr) {
    _syscall1(SYS_FREE, ptr);
}
