#ifndef __INCLUDE_KERNEL_SYNC_H
#define __INCLUDE_KERNEL_SYNC_H

#include <stdint.h>
#include <list.h>

struct semaphore {
    uint8 value;
    struct list waiters;
};

struct lock {
    struct taskStruct *holder;
    struct semaphore semaphore;
    uint32 holderRepeatedCnt;           // count of holder requesting the lock
};

struct condition {
    struct list waiters;
};

void semaphoreInit(struct semaphore *sem, uint8 value);
void semaphoreDown(struct semaphore *sem);
void semaphoreUp(struct semaphore *sem);

void lockInit(struct lock *pLock);
void lockAcquire(struct lock *pLock);
void lockRelease(struct lock *pLock);

void conditionInit(struct condition *cond);
void conditionWait(struct condition *cond, struct lock *plock);
void conditionSignal(struct condition *cond);
void conditionSignalAll(struct condition *cond);

#endif
