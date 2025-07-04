#ifndef __INCLUDE_DEVICE_IOQUEUE_H
#define __INCLUDE_DEVICE_IOQUEUE_H

#include <stdint.h>
#include <kernel/sync.h>

#define BUFSIZE 64

// ioqueue is a circular queue and thread-safe.
struct ioqueue {
    struct lock lock;
    struct condition condNotEmpty;
    struct condition condNotFull;

    char buf[BUFSIZE];

    uint32 head;
    uint32 tail;
};

void ioqueueInit(struct ioqueue *ioq);
char ioqueueGetchar(struct ioqueue *ioq);
void ioqueuePutchar(struct ioqueue *ioq, char ch);

#endif
