#include <device/ioqueue.h>
#include <kernel/interrupt.h>
#include <kernel/debug.h>

void ioqueueInit(struct ioqueue *ioq) {
    lockInit(&ioq->lock);
    conditionInit(&ioq->condNotEmpty);
    conditionInit(&ioq->condNotFull);

    ioq->head = 0;
    ioq->tail = 0;
}

static inline uint32 nextPos(uint32 pos) {
    return (pos + 1) % BUFSIZE;
}

static bool ioqueueFull(struct ioqueue *ioq) {
    return nextPos(ioq->tail) == ioq->head;
}

static bool ioqueueEmpty(struct ioqueue *ioq) {
    return ioq->tail == ioq->head;
}

char ioqueueGetchar(struct ioqueue *ioq) {
    lockAcquire(&ioq->lock);

    while (ioqueueEmpty(ioq)) {
        conditionWait(&ioq->condNotEmpty, &ioq->lock);
    }

    char ch = ioq->buf[ioq->head];
    ioq->head = nextPos(ioq->head);

    conditionSignal(&ioq->condNotFull);
    lockRelease(&ioq->lock);

    return ch;
}

void ioqueuePutchar(struct ioqueue *ioq, char ch) {
    lockAcquire(&ioq->lock);

    while (ioqueueFull(ioq)) {
        conditionWait(&ioq->condNotFull, &ioq->lock);
    }

    ioq->buf[ioq->tail] = ch;
    ioq->tail = nextPos(ioq->tail);

    conditionSignal(&ioq->condNotEmpty);
    lockRelease(&ioq->lock);
}
