#ifndef __INCLUDE_KERNEL_MEMORY_H
#define __INCLUDE_KERNEL_MEMORY_H

#include <kernel/bitmap.h>
#include <stdint.h>

struct virtualAddr {
    struct bitmap vaddrBitmap;      // bitmap used by virtual address
    uint32        vaddrStart;       // start address of virtual address
};

void memInit();

#endif
