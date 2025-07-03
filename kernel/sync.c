#include <kernel/sync.h>
#include <kernel/interrupt.h>
#include <list.h>
#include <stdint.h>
#include <string.h>
#include <kernel/debug.h>
#include <kernel/thread.h>

void semaphoreInit(struct semaphore *sem, uint8 value) {
    sem->value = value;
    listInit(&sem->waiters);
}

void lockInit(struct lock *pLock) {
    pLock->holder = NULL;
    pLock->holderRepeatedCnt = 0;
    semaphoreInit(&pLock->semaphore, 1);
}

// semaphoreDown decreases the value in sem by 1.
// if the value is 0, current thread will be blocked.
void semaphoreDown(struct semaphore *sem) {
    enum intrStatus oldStatus = intrDisable();

    while (sem->value == 0) {
        ASSERT(!listExist(&sem->waiters, &threadCurrent()->generalTag));
        if (listExist(&sem->waiters, &threadCurrent()->generalTag)) {
            PANIC("semaphoreDown: thread blocked has been in waiter list.\n");
        }
        listAppend(&sem->waiters, &threadCurrent()->generalTag);
        threadBlock(TASK_BLOCKED);
    }

    sem->value -= 1;

    intrSetStatus(oldStatus);
}

// semaphoreUp increases the value in sem by 1.
// if the sem->waiters isn's empty, it will also wake up a thread in sem->waiters.
void semaphoreUp(struct semaphore *sem) {
    enum intrStatus oldStatus = intrDisable();

    if (!listEmpty(&sem->waiters)) {
        struct listNode *threadTag = listPop(&sem->waiters);
        struct taskStruct *threadBlocked = elem2entry(struct taskStruct, generalTag, threadTag);
        threadUnblock(threadBlocked);
    }

    sem->value += 1;

    intrSetStatus(oldStatus);
}

void lockAcquire(struct lock *pLock) {
    // reentrant case
    if (pLock->holder == threadCurrent()) {
        pLock->holderRepeatedCnt += 1;
        return;
    }

    semaphoreDown(&pLock->semaphore);
    pLock->holder = threadCurrent();
    pLock->holderRepeatedCnt = 1;
}

void lockRelease(struct lock *pLock) {
    ASSERT(pLock->holder == threadCurrent());

    pLock->holderRepeatedCnt -= 1;
    if (pLock->holderRepeatedCnt == 0) {
        pLock->holder = NULL;
        semaphoreUp(&pLock->semaphore);
    }
}
