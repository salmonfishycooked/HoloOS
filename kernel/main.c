#include <kernel/print.h>
#include <kernel/init.h>
#include <kernel/debug.h>
#include <kernel/memory.h>
#include <kernel/thread.h>
#include <kernel/interrupt.h>
#include <device/console.h>

void work(void *arg);

int main() {
    clearScreen();
    puts("HoloOS is booting...\n");

    initAll();

    // threadStart("kthread 1", 32, work, "A ");
    // threadStart("kthread 2", 8, work, "B ");

    intrEnable();
    // while (1) { consolePuts("Main "); }
    while (1) {}
}

void work(void *arg) {
    char *p = (char *) arg;
    while (1) { consolePuts(p); }
}
