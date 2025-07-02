#include <kernel/print.h>
#include <kernel/init.h>
#include <kernel/debug.h>
#include <kernel/memory.h>
#include <kernel/thread.h>
#include <kernel/interrupt.h>

void work(void *arg);

int main() {
    clearScreen();
    puts("HoloOS is booting...\n");

    initAll();

    threadStart("kthread 1", 1, work, "A ");
    threadStart("kthread 2", 1, work, "B ");

    intrEnable();
    while (1) { puts("Main "); }
}

void work(void *arg) {
    char *p = (char *) arg;
    while (1) { puts(p); }
}
