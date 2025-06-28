#include <kernel/print.h>
#include <kernel/init.h>
#include <kernel/debug.h>
#include <kernel/memory.h>

int main() {
    clearScreen();
    puts("HoloOS is booting...\n");

    initAll();

    // asm volatile ("sti");

    // ASSERT(0 == 1);

    void *addr = getKernelPages(3);
    puts("\nI allocated 3 pages from kernel pool, its start addr is 0x");
    putint((uint32) addr);
    putchar('\n');

    while (1) {}
}
