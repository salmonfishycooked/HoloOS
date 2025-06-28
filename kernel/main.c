#include <kernel/print.h>
#include <kernel/init.h>
#include <kernel/debug.h>

int main() {
    clearScreen();

    puts("HoloOS is booting...\n");

    initAll();

    // asm volatile ("sti");

    // ASSERT(0 == 1);

    while (1) {}
}
