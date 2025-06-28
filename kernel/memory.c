#include <kernel/memory.h>
#include <kernel/print.h>
#include <kernel/debug.h>
#include <stdint.h>
#include <string.h>

#define PG_SIZE 4096

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
};

static struct pool kernelPool, userPool;
static struct virtualAddr kernelVaddr;


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
        // TODO: PF_USER
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
// and set all of bits to 0.
// return start virtual address of pgCnt pages if ok;
// return NULL if not ok.
void *getKernelPages(uint32 pgCnt) {
    void *vaddr = mallocPage(PF_KERNEL, pgCnt);
    if (vaddr != NULL) {
        memset(vaddr, 0, pgCnt * PG_SIZE);
    }

    return vaddr;
}


static void memPoolInit(uint32 memCapacity) {
    puts("  memory pool init starts.\n");

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

    puts("memory init done.\n");
}
