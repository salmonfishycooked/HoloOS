#ifndef __INCLUDE_KERNEL_SYNC_H
#define __INCLUDE_KERNEL_SYNC_H

#include <stdint.h>
#include <list.h>
#include <kernel/interrupt.h>

#define cpuRelax()    asm volatile("pause" ::: "memory")

struct spinlock {
    int occupied;
};

struct lock {
    struct taskStruct *holder;
    struct list waiters;
    struct spinlock waitersLock;
};

struct condition {
    struct list waiters;
};

struct semaphore {
    int value;
    struct lock lock;
    struct condition cond;
};

void spinlockInit(struct spinlock *lock);
void spinlockAcquire(struct spinlock *lock);
void spinlockRelease(struct spinlock *lock);

void lockInit(struct lock *pLock);
void lockAcquire(struct lock *pLock);
void lockRelease(struct lock *pLock);

void conditionInit(struct condition *cond);
void conditionWait(struct condition *cond, struct lock *plock);
void conditionSignal(struct condition *cond);
void conditionSignalAll(struct condition *cond);

void semaphoreInit(struct semaphore *sem, int value);
void semaphoreDown(struct semaphore *sem);
void semaphoreUp(struct semaphore *sem);

#endif
