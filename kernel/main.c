#include <kernel/print.h>
#include <kernel/init.h>

int main() {
    clearScreen();

    puts("HoloOS is booting...\n");

    initAll();

    asm volatile ("sti");

    while (1) {}
}
