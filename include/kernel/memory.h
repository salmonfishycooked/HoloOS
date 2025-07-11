#ifndef __INCLUDE_KERNEL_MEMORY_H
#define __INCLUDE_KERNEL_MEMORY_H

#include <kernel/bitmap.h>
#include <stdint.h>
#include <list.h>


#define DESC_CNT        7


struct virtualAddr {
    struct bitmap vaddrBitmap;      // bitmap used by virtual address
    uint32        vaddrStart;       // start address of virtual address
};


// used to determine which pool should be used.
enum poolFlags {
    PF_KERNEL = 1,
    PF_USER   = 2
};


struct memBlock {
    struct listNode freeNode;
};


// memory block descriptor
struct memBlockDesc {
    uint32      blockSize;                  // size of a block
    uint32      blocksPerArena;             // number of blocks that an arena can contain
    struct list freeList;                   // list of free blocks
};


// attributes of pte
#define PG_P_1      1               // present bit
#define PG_P_0      0
#define PG_RW_R     0               // read/write bit
#define PG_RW_W     2
#define PG_US_S     0               // U/S bit
#define PG_US_U     4

void memInit();

void *mallocPage(enum poolFlags pf, uint32 pgCnt);
void *getKernelPages(uint32 pgCnt);
void *getUserPages(uint32 pgCnt);
void *getPage(enum poolFlags pf, uint32 vaddr);
uint32 v2p(uint32 vaddr);
void *sysMalloc(uint32 size);
void sysFree(void *ptr);

void blockDescInit(struct memBlockDesc *descArr);

#endif
