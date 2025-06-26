#include <kernel/print.h>
#include <kernel/init.h>

void main() {
    clearScreen();

    puts("I am HoloOS\n");

    initAll();

    asm volatile ("sti");

    while (1) {}
}
