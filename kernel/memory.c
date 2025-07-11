#include <kernel/memory.h>
#include <kernel/print.h>
#include <kernel/debug.h>
#include <kernel/global.h>
#include <kernel/sync.h>
#include <stdint.h>
#include <kernel/thread.h>
#include <string.h>
#include <kernel/debug.h>


// 0xc009f000 is the top of stack of kernel process,
// and 0xc009e000 is the pcb of kernel process,
// so we use 4 * 4KB for bitmap. (can manage 4 * 128MB memory)
#define MEM_BITMAP_BASE 0xc009a000

// 0xc0000000 is the start address of kernel,
// and we jump over first lower 1MB memory.
// This is start virtual address of heap of kernel process.
#define K_HEAP_START    0xc0100000

#define PDE_IDX(addr) ((addr & 0xffc00000) >> 22)
#define PTE_IDX(addr) ((addr & 0x003ff000) >> 12)


struct pool {
    struct bitmap poolBitmap;
    uint32        paddrStart;
    uint32        poolSize;         // unit: byte
    struct lock   lock;
};


struct arena {
    struct memBlockDesc   *desc;    // memBlockDesc related to this arena
    uint32                cnt;      // number of blocks if flag is true,
                                    // otherwise number of page frames
    bool                  large;
};


struct memBlockDesc kBlockDescs[DESC_CNT];

static struct pool kernelPool, userPool;
static struct virtualAddr kernelVaddr;


void blockDescInit(struct memBlockDesc *descArr) {
    uint32 blockSize = 16;

    for (uint32 i = 0; i < DESC_CNT; i++) {
        descArr[i].blockSize = blockSize;
        descArr[i].blocksPerArena = (PG_SIZE - sizeof(struct arena)) / blockSize;
        listInit(&descArr[i].freeList);

        blockSize += blockSize;
    }
}


// arena2block returns the address of the idx-th block in arena a.
static struct memBlock *arena2block(struct arena *a, uint32 idx) {
    return (struct memBlock *) ((uint32) a + sizeof(struct arena) + idx * a->desc->blockSize);
}


// block2arena returns the address of the arena related to b.
static struct arena *block2arena(struct memBlock *b) {
    return (struct arena *) ((uint32) b & 0xfffff000);
}


// sysMalloc allocates size bytes in heap.
void *sysMalloc(uint32 size) {
    enum poolFlags pf;
    struct pool *memPool;
    uint32 poolSize;
    struct memBlockDesc *descs;
    struct taskStruct *curThread = threadCurrent();

    // determine what memory pool we should use
    if (curThread->pageDir == NULL) {
        // case: kernel thread
        pf = PF_KERNEL;
        poolSize = kernelPool.poolSize;
        memPool = &kernelPool;
        descs = kBlockDescs;
    } else {
        // case: user thread
        pf = PF_USER;
        poolSize = userPool.poolSize;
        memPool = &userPool;
        descs = curThread->uBlockDesc;
    }

    if (size <= 0 || size >= poolSize) { return NULL; }

    struct arena *a;
    struct memBlock *b;

    lockAcquire(&memPool->lock);

    if (size > 1024) {
        // allocates page frame directly.
        uint32 pageCnt = (size + sizeof(struct arena) + PG_SIZE - 1) / PG_SIZE;
        a = mallocPage(pf, pageCnt);
        if (a == NULL) {
            lockRelease(&memPool->lock);
            return NULL;
        }

        memset(a, 0, pageCnt * PG_SIZE);
        a->desc = NULL;
        a->cnt = pageCnt;
        a->large = true;

        lockRelease(&memPool->lock);
        return (void *) (a + 1);
    }

    // case: 0 < size < 1024
    // allocates block from arena.
    uint8 descIdx;
    for (descIdx = 0; descIdx < DESC_CNT; descIdx++) {
        if (size <= descs[descIdx].blockSize) { break; }
    }

    // if there's no arena remaining, we create a new arena.
    if (listEmpty(&descs[descIdx].freeList)) {
        a = mallocPage(pf, 1);
        if (a == NULL) {
            lockRelease(&memPool->lock);
            return NULL;
        }

        memset(a, 0, PG_SIZE);
        a->desc = &descs[descIdx];
        a->large = false;
        a->cnt = descs[descIdx].blocksPerArena;

        // split up the arena.
        enum intrStatus oldStatus = intrDisable();
        for (uint32 blockIdx = 0; blockIdx < descs[descIdx].blocksPerArena; blockIdx++) {
            b = arena2block(a, blockIdx);
            ASSERT(!listExist(&a->desc->freeList, &b->freeNode));
            listAppend(&a->desc->freeList, &b->freeNode);
        }
        intrSetStatus(oldStatus);
    }

    b = elem2entry(struct memBlock, freeNode, listPop(&(descs[descIdx].freeList)));
    memset(b, 0, descs[descIdx].blockSize);

    a = block2arena(b);
    a->cnt -= 1;

    lockRelease(&memPool->lock);
    return (void *) b;
}


// vaddrGet finds pgCnt pages available to use.
// return start address if ok;
// return NULL if not ok.
static void *vaddrGet(enum poolFlags pf, uint32 pgCnt) {
    int vaddrStart = 0, bitIdxStart = -1;

    if (pf == PF_KERNEL) {
        bitIdxStart = bitmapScan(&kernelVaddr.vaddrBitmap, pgCnt);
        if (bitIdxStart == -1) { return NULL; }

        for (uint32 i = 0; i < pgCnt; ++i) {
            bitmapSet(&kernelVaddr.vaddrBitmap, bitIdxStart + i, 1);
        }
        vaddrStart = kernelVaddr.vaddrStart + bitIdxStart * PG_SIZE;

    } else {
        // case PF_USER
        struct taskStruct *cur = threadCurrent();
        bitIdxStart = bitmapScan(&cur->userVaddr.vaddrBitmap, pgCnt);
        if (bitIdxStart == -1) { return NULL; }

        for (uint32 i = 0; i < pgCnt; ++i) {
            bitmapSet(&cur->userVaddr.vaddrBitmap, bitIdxStart + i, 1);
        }
        vaddrStart = cur->userVaddr.vaddrStart + bitIdxStart * PG_SIZE;

        // (0xc0000000 - PGSIZE) ~ 0xc0000000 is reserved for stack of user process.
        ASSERT((uint32) vaddrStart < (0xc0000000 - PG_SIZE))
    }

    return (void *) vaddrStart;
}


// pdePtr returns corresponding pde address of vaddr.
static uint32 *pdePtr(uint32 vaddr) {
    // use 0xfffff000 to access the physical address of pde
    return (uint32 *) (0xfffff000 + PDE_IDX(vaddr) * 4);
}


// ptePtr returns corresponding pte address of vaddr.
static uint32 *ptePtr(uint32 vaddr) {
    // use 0xffc00000 to access the physical address of pde,
    // and use (vaddr & 0xffc00000) >> 10 to access its real pde,
    // then PDE_IDX(vaddr) * 4 to access the physical address of its real pte.
    return (uint32 *) (0xffc00000 + ((vaddr & 0xffc00000) >> 10) + PTE_IDX(vaddr) * 4);
}


// palloc allocates 1 page frame from memPool.
// return physical start address of the page frame if ok;
// return NULL if not ok.
static void *palloc(struct pool *memPool) {
    int bitIdx = bitmapScan(&memPool->poolBitmap, 1);
    if (bitIdx == -1) { return NULL; }

    bitmapSet(&memPool->poolBitmap, bitIdx, 1);
    uint32 pagePhyAddr = (bitIdx * PG_SIZE) + memPool->paddrStart;

    return (void *) pagePhyAddr;
}


// pageTableAdd maps vaddr to pagePhyAddr in pte
static void pageTableAdd(void *_vaddr, void *_pagePhyAddr) {
    uint32 vaddr = (uint32) _vaddr, pagePhyAddr = (uint32) _pagePhyAddr;
    uint32 *pde = pdePtr(vaddr), *pte = ptePtr(vaddr);

    // if pde doesn't exist, allocate a page frame to create it,
    // and clear its related pte.
    if (!(*pde & 0x00000001)) {
        uint32 pdePhyAddr = (uint32) palloc(&kernelPool);
        *pde = pdePhyAddr | PG_US_U | PG_RW_W | PG_P_1;
        memset((void *) ((uint32) pte & 0xfffff000), 0, PG_SIZE);
    }

    ASSERT(!(*pte & 0x00000001));

    // write mapping data into related pte
    *pte = pagePhyAddr | PG_US_U | PG_RW_W | PG_P_1;
}


// mallocPage allocates pgCnt pages from corresponding pool (according to pf),
// and create related mapping in page table.
// return start virtual address of pgCnt pages if ok;
// return NULL if not ok;
void *mallocPage(enum poolFlags pf, uint32 pgCnt) {
    ASSERT(pgCnt > 0 && pgCnt < 3840);

    void *vaddrStart = vaddrGet(pf, pgCnt);
    if (vaddrStart == NULL) { return NULL; }

    uint32 vaddr = (uint32) vaddrStart;
    struct pool *memPool = pf & PF_KERNEL ? &kernelPool : &userPool;

    for (uint32 i = 0; i < pgCnt; i++) {
        void *pagePhyAddr = palloc(memPool);
        if (pagePhyAddr == NULL) {
            // TODO: reclaim allocated pages.
            return NULL;
        }

        pageTableAdd((void *) vaddr, pagePhyAddr);
        vaddr += PG_SIZE;
    }

    return vaddrStart;
}


// getKernelPages allocates pgCnt available pages,
// and sets all of bits to 0.
// return start virtual address of pgCnt pages if ok;
// return NULL if not ok.
void *getKernelPages(uint32 pgCnt) {
    void *vaddr = mallocPage(PF_KERNEL, pgCnt);
    if (vaddr != NULL) {
        memset(vaddr, 0, pgCnt * PG_SIZE);
    }

    return vaddr;
}


// getUserPages allocates pgCnt available pages,
// and sets all of bits to 0.
// return start virtual address of pgCnt pages if ok;
// return NULL if not ok.
void *getUserPages(uint32 pgCnt) {
    lockAcquire(&userPool.lock);

    void *vaddr = mallocPage(PF_USER, pgCnt);
    if (vaddr != NULL) {
        memset(vaddr, 0, pgCnt * PG_SIZE);
    }

    lockRelease(&userPool.lock);

    return vaddr;
}


// getPage finds an available physical page frame in pf and relates vaddr to it.
// only gets 1 page.
void *getPage(enum poolFlags pf, uint32 vaddr) {
    struct pool *memPool = pf & PF_KERNEL ? &kernelPool : &userPool;
    lockAcquire(&memPool->lock);

    struct taskStruct *cur = threadCurrent();
    int32 bitIdx = -1;

    if (cur->pageDir != NULL && pf == PF_USER) {
        // case: user process
        bitIdx = (vaddr - cur->userVaddr.vaddrStart) / PG_SIZE;
        ASSERT(bitIdx > 0);
        bitmapSet(&cur->userVaddr.vaddrBitmap, bitIdx, 1);
    } else if (cur->pageDir == NULL && pf == PF_KERNEL) {
        // case: kernel process
        bitIdx = (vaddr - kernelVaddr.vaddrStart) / PG_SIZE;
        ASSERT(bitIdx > 0);
        bitmapSet(&kernelVaddr.vaddrBitmap, bitIdx, 1);
    } else {
        PANIC("getPage: not allow kernel to allocate user space or user to allocate kernel space!\n");
    }

    void *pagePhyAddr = palloc(memPool);
    if (pagePhyAddr == NULL) { return NULL; }

    pageTableAdd((void *) vaddr, pagePhyAddr);

    lockRelease(&memPool->lock);

    return (void *) vaddr;
}


// v2p returns physical address related to vaddr
uint32 v2p(uint32 vaddr) {
    uint32 *pte = ptePtr(vaddr);

    return (*pte & 0xfffff000) + (vaddr & 0x00000fff);
}


static void memPoolInit(uint32 memCapacity) {
    puts("  memory pool init starts.\n");

    lockInit(&kernelPool.lock);
    lockInit(&userPool.lock);

    // pde: we have used 1 page
    // pte: we have usde 255 page
    uint32 pageTableSize = PG_SIZE * 256;
    uint32 usedMemory = 0x100000 + pageTableSize;
    uint32 freeMemory = memCapacity - usedMemory;

    uint16 allFreePages = freeMemory / PG_SIZE;
    uint16 kernelFreePages = allFreePages / 2;
    uint16 userFreePages = allFreePages - kernelFreePages;

    // bitmap length respectively
    uint32 kbmLength = kernelFreePages / 8;
    uint32 ubmLength = userFreePages / 8;

    uint32 kPhyStart = usedMemory;
    uint32 uPhyStart = kPhyStart + kernelFreePages * PG_SIZE;

    kernelPool.paddrStart = kPhyStart;
    kernelPool.poolSize = kernelFreePages * PG_SIZE;
    kernelPool.poolBitmap.size = kbmLength;
    kernelPool.poolBitmap.bits = (uint8 *) MEM_BITMAP_BASE;
    bitmapInit(&kernelPool.poolBitmap);

    userPool.paddrStart = uPhyStart;
    userPool.poolSize = userFreePages * PG_SIZE;
    userPool.poolBitmap.size = ubmLength;
    userPool.poolBitmap.bits = (uint8 *) (MEM_BITMAP_BASE + kbmLength);
    bitmapInit(&userPool.poolBitmap);

    kernelVaddr.vaddrBitmap.bits = (uint8 *) (MEM_BITMAP_BASE + kbmLength + ubmLength);
    kernelVaddr.vaddrBitmap.size = kbmLength;
    kernelVaddr.vaddrStart = K_HEAP_START;
    bitmapInit(&kernelVaddr.vaddrBitmap);

    // output memory pool information
    puts("    kernel pool bitmap start vaddress: 0x");
    putint((int) kernelPool.poolBitmap.bits);
    putchar('\n');
    puts("    kernel pool physical start address: 0x");
    putint(kernelPool.paddrStart);
    putchar('\n');

    puts("    user pool bitmap start vaddress: 0x");
    putint((int) userPool.poolBitmap.bits);
    putchar('\n');
    puts("    user pool physical start address: 0x");
    putint(userPool.paddrStart);
    putchar('\n');

    puts("  memory pool init done.\n");
}

void memInit() {
    puts("memory init starts.\n");

    uint32 memCapacity = *((uint32 *) 0xb06);
    memPoolInit(memCapacity);

    blockDescInit(kBlockDescs);

    puts("memory init done.\n");
}
