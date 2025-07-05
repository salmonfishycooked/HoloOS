#include <kernel/sync.h>
#include <kernel/interrupt.h>
#include <list.h>
#include <stdint.h>
#include <string.h>
#include <kernel/debug.h>
#include <kernel/thread.h>
#include <kernel/print.h>
#include <cas.h>

void spinlockInit(struct spinlock *lock) {
    lock->occupied = 0;
}

void spinlockAcquire(struct spinlock *lock) {
    while (!compareAndSwap32(&lock->occupied, 0, 1)) {
        cpuRelax();
    }
}

void spinlockRelease(struct spinlock *lock) {
    lock->occupied = 0;
}

void lockInit(struct lock *lock) {
    lock->holder = NULL;
    listInit(&lock->waiters);
    spinlockInit(&lock->waitersLock);
}

void lockAcquire(struct lock *lock) {
    enum intrStatus stat = INTR_ON;
    while (!compareAndSwap32((int *) &lock->holder, NULL, (int) threadCurrent())) {
        spinlockAcquire(&lock->waitersLock);
        stat = intrDisable();
        listAppend(&lock->waiters, &threadCurrent()->generalTag);
        threadSetStatus(TASK_BLOCKED);
        spinlockRelease(&lock->waitersLock);

        if (compareAndSwap32((int *) &lock->holder, NULL, (int) threadCurrent())) {
            spinlockAcquire(&lock->waitersLock);
            listRemove(&threadCurrent()->generalTag);
            spinlockRelease(&lock->waitersLock);

            threadSetStatus(TASK_RUNNING);
            intrSetStatus(stat);
            break;
        }

        schedule();
        intrSetStatus(stat);
    }
}

void lockRelease(struct lock *lock) {
    ASSERT(lock->holder == threadCurrent());

    lock->holder = NULL;

    spinlockAcquire(&lock->waitersLock);
    if (!listEmpty(&lock->waiters)) {
        struct listNode *threadTag = listPop(&lock->waiters);
        spinlockRelease(&lock->waitersLock);

        struct taskStruct *thread = elem2entry(struct taskStruct, generalTag, threadTag);
        if (thread->status == TASK_BLOCKED) {
            threadUnblock(thread);
        }

    } else {
        spinlockRelease(&lock->waitersLock);
    }
}

void conditionInit(struct condition *cond) {
    listInit(&cond->waiters);
}

void conditionWait(struct condition *cond, struct lock *lock) {
    enum intrStatus oldStatus = intrDisable();

    struct taskStruct *curThread = threadCurrent();
    listAppend(&cond->waiters, &curThread->generalTag);
    threadSetStatus(TASK_BLOCKED);
    lockRelease(lock);

    schedule();
    lockAcquire(lock);

    intrSetStatus(oldStatus);
}

void conditionSignal(struct condition *cond) {
    if (listEmpty(&cond->waiters)) { return; }

    struct listNode *threadTag = listPop(&cond->waiters);
    struct taskStruct *thread = elem2entry(struct taskStruct, generalTag, threadTag);
    threadUnblock(thread);
}

void conditionSignalAll(struct condition *cond) {
    while (!listEmpty(&cond->waiters)) {
        conditionSignal(cond);
    }
}

void semaphoreInit(struct semaphore *sem, int value) {
    sem->value = value;
    lockInit(&sem->lock);
    conditionInit(&sem->cond);
}

// semaphoreDown decreases the value in sem by 1.
// if the value is 0, current thread will be blocked.
void semaphoreDown(struct semaphore *sem) {
    lockAcquire(&sem->lock);

    while (sem->value == 0) {
        conditionWait(&sem->cond, &sem->lock);
    }
    sem->value -= 1;

    lockRelease(&sem->lock);
}

// semaphoreUp increases the value in sem by 1.
// if the sem->waiters isn's empty, it will also wake up a thread in sem->waiters.
void semaphoreUp(struct semaphore *sem) {
    lockAcquire(&sem->lock);

    sem->value += 1;
    conditionSignal(&sem->cond);

    lockRelease(&sem->lock);
}
