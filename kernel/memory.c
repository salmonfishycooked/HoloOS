#include <kernel/memory.h>
#include <kernel/print.h>
#include <stdint.h>

#define PG_SIZE 4096

// 0xc009f000 is the top of stack of kernel process,
// and 0xc009e000 is the pcb of kernel process,
// so we use 4 * 4KB for bitmap. (can manage 4 * 128MB memory)
#define MEM_BITMAP_BASE 0xc009a000

// 0xc0000000 is the start address of kernel,
// and we jump over first lower 1MB memory.
// This is start virtual address of heap of kernel process.
#define K_HEAP_START    0xc0100000

struct pool {
    struct bitmap poolBitmap;
    uint32        paddrStart;
    uint32        poolSize;         // unit: byte
};

struct pool kernelPool, userPool;
struct virtualAddr kernelVaddr;

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
