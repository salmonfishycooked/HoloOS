#include <kernel/thread.h>
#include <kernel/global.h>
#include <kernel/memory.h>
#include <kernel/interrupt.h>
#include <kernel/debug.h>
#include <kernel/print.h>
#include <list.h>
#include <stdint.h>
#include <string.h>

struct taskStruct *mainThread;
struct list threadReadyList;
struct list threadAllList;
static struct listNode *threadTag;


extern void switchTo(struct taskStruct *cur, struct taskStruct *next);


static void kernelThread(threadFunc func, void *arg) {
    intrEnable();
    func(arg);
}

// threadCurrent returns the address of task struct of current thread.
struct taskStruct *threadCurrent() {
    uint32 esp;
    asm volatile ("mov %%esp, %0" : "=g"(esp));

    return (struct taskStruct *) (esp & 0xfffff000);
}

// threadCreate reserves room for interrupt stack and thread stack,
// and initializes threadStack struct.
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

// threadInit initializes task of task struct.
static void threadInit(struct taskStruct *task, char *name, int priority) {
    task->status = TASK_READY;
    if (task == mainThread) { task->status = TASK_RUNNING; }

    strcpy(task->name, name);
    task->priority = priority;
    task->selfKStack = (uint32 *) ((uint32) task + PG_SIZE);
    task->ticks = priority;
    task->elapsedTicks = 0;
    task->pageDir = NULL;
    task->stackMagic = STACK_MAGIC;
}

// threadStart starts a new kernel thread.
struct taskStruct *threadStart(char *name, int priority, threadFunc func, void *arg) {
    struct taskStruct *task = getKernelPages(1);

    threadInit(task, name, priority);
    threadCreate(task, func, arg);

    ASSERT(!listExist(&threadReadyList, &task->generalTag));
    listAppend(&threadReadyList, &task->generalTag);

    ASSERT(!listExist(&threadAllList, &task->allListTag));
    listAppend(&threadAllList, &task->allListTag);

    // asm volatile ("movl %0, %%esp;"
    //               "pop %%ebp; pop %%ebx; pop %%edi; pop %%esi;"
    //               "ret;"
    //               : : "g"(task->selfKStack): "memory"
    // );

    return task;
}

// makeMainThread makes the kernel thread a complete thread.
static void makeMainThread() {
    mainThread = threadCurrent();
    threadInit(mainThread, "main", 31);

    ASSERT(!listExist(&threadAllList, &mainThread->allListTag));
    listAppend(&threadAllList, &mainThread->allListTag);
}

// threadBlock used to block current thread.
// the status of current thread will be set to stat;
void threadBlock(enum taskStatus stat) {
    ASSERT((stat == TASK_BLOCKED) || (stat == TASK_RUNNING) || (stat == TASK_HANGING));

    enum intrStatus oldStatus = intrDisable();

    struct taskStruct *curThread = threadCurrent();
    curThread->status = stat;

    schedule();

    intrSetStatus(oldStatus);
}

// threadUnblock wakes up pthread passed in.
void threadUnblock(struct taskStruct *pthread) {
    enum intrStatus oldStatus = intrDisable();

    ASSERT((pthread->status == TASK_BLOCKED) || (pthread->status == TASK_WAITING) ||
           (pthread->status == TASK_HANGING));

    if (pthread->status != TASK_READY) {
        ASSERT(!listExist(&threadReadyList, &pthread->generalTag));
        if (listExist(&threadReadyList, &pthread->generalTag)) {
            PANIC("thread unblock: blocked thread already in ready list.\n");
        }
        listPush(&threadReadyList, &pthread->generalTag);
        pthread->status = TASK_READY;
    }

    intrSetStatus(oldStatus);
}

// schedule used to schedule kernel threads.
void schedule() {
    ASSERT(intrGetStatus() == INTR_OFF);

    struct taskStruct *cur = threadCurrent();
    if (cur->status == TASK_RUNNING) {
        // case: ticks has been run out.
        ASSERT(!listExist(&threadReadyList, &cur->generalTag));
        listAppend(&threadReadyList, &cur->generalTag);

        cur->ticks = cur->priority;
        cur->status = TASK_READY;
    } else {
        // TODO: case: this thread is needed to be scheduled for some events.
    }

    ASSERT(!listEmpty(&threadReadyList));

    threadTag = NULL;
    threadTag = listPop(&threadReadyList);
    struct taskStruct *next = elem2entry(struct taskStruct, generalTag, threadTag);
    next->status = TASK_RUNNING;

    switchTo(cur, next);
}

// threadSupportInit initializes the environment necessary to kernel thread support.
void threadSupportInit() {
    puts("thread support is initializing...\n");

    listInit(&threadReadyList);
    listInit(&threadAllList);

    makeMainThread();

    puts("thread support init done!\n");
}
