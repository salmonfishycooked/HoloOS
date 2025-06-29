#ifndef __INCLUDE_KERNEL_THREAD_H
#define __INCLUDE_KERNEL_THREAD_H

#include <stdint.h>

#define STACK_MAGIC 0x87359642

typedef void (*threadFunc) (void *);

enum taskStatus {
    TASK_RUNNING,
    TASK_READY,
    TASK_BLOCKED,
    TASK_WAITING,
    TASK_HANGING,
    TASK_DIED 
};

struct intrStack {
    uint32 vecNo;               // interrupt number
    uint32 edi;
    uint32 esi;
    uint32 ebp;
    uint32 espDummy;
    uint32 ebx;
    uint32 edx;
    uint32 ecx;
    uint32 eax;
    uint32 gs;
    uint32 fs;
    uint32 es;
    uint32 ds;

    uint32 errcode;
    void (*eip) (void);
    uint32 cs;
    uint32 eflags;
    void *esp;
    uint32 ss;
};

struct threadStack {
    uint32 ebp;
    uint32 ebx;
    uint32 edi;
    uint32 esi;

    void (*eip) (threadFunc func, void *arg);

    // used for kernel thread being scheduled at first time.
    void (*unusedRetAddr);
    threadFunc func;
    void* arg;
};

struct taskStruct {
    uint32 *selfKStack;            // address of kernel stack of current thread
    enum taskStatus status;
    uint8 priority;
    char name[16];

    // boundary mark of current kernel stack,
    // for protecting from overflow of stack.
    uint32 stackMagic;
};

struct taskStruct *threadStart(char *name, int priority, threadFunc, void *arg);

#endif
