#include <kernel/print.h>
#include <kernel/init.h>
#include <kernel/debug.h>
#include <kernel/memory.h>
#include <kernel/thread.h>

void kthread1(void *arg);

int main() {
    clearScreen();
    puts("HoloOS is booting...\n");

    initAll();

    // asm volatile ("sti");

    // ASSERT(0 == 1);

    // void *addr = getKernelPages(3);
    // puts("\nI allocated 3 pages from kernel pool, its start addr is 0x");
    // putint((uint32) addr);
    // putchar('\n');

    threadStart("kthread 1", 31, kthread1, "kthread 1 ok!");

    while (1) {}
}

void kthread1(void *arg) {
    char *p = (char *) arg;
    puts(p);

    while (1) {}
}
