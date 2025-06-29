#include <kernel/thread.h>
#include <kernel/global.h>
#include <kernel/memory.h>
#include <stdint.h>
#include <string.h>

static void kernelThread(threadFunc func, void *arg) {
    func(arg);
}

static void threadCreate(struct taskStruct *task, threadFunc func, void *arg) {
    // reserved space for intrrupt stack and thread stack
    task->selfKStack -= sizeof(struct intrStack);
    task->selfKStack -= sizeof(struct threadStack);

    struct threadStack *kthreadStack = (struct threadStack*) task->selfKStack;
    kthreadStack->eip = kernelThread;
    kthreadStack->func = func;
    kthreadStack->arg = arg;
    kthreadStack->ebp = 0;
    kthreadStack->ebx = 0;
    kthreadStack->esi = 0;
    kthreadStack->edi = 0;
}

static void threadInit(struct taskStruct *task, char *name, int priority) {
    strcpy(task->name, name);
    task->status = TASK_RUNNING;
    task->priority = priority;
    task->selfKStack = (uint32 *) ((uint32) task + PG_SIZE);
    task->stackMagic = STACK_MAGIC;
}

// threadStart starts a new kernel thread.
struct taskStruct *threadStart(char *name, int priority, threadFunc func, void *arg) {
    struct taskStruct *task = getKernelPages(1);

    threadInit(task, name, priority);
    threadCreate(task, func, arg);

    asm volatile ("movl %0, %%esp;"
                  "pop %%ebp; pop %%ebx; pop %%edi; pop %%esi;"
                  "ret;"
                  : : "g"(task->selfKStack): "memory"
    );

    return task;
}
