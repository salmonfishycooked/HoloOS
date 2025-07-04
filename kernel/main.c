#include <kernel/print.h>
#include <kernel/init.h>
#include <kernel/debug.h>
#include <kernel/memory.h>
#include <kernel/thread.h>
#include <kernel/interrupt.h>
#include <device/console.h>
#include <device/keyboard.h>
#include <device/ioqueue.h>

void work(void *arg);

int main() {
    clearScreen();
    puts("HoloOS is booting...\n");

    initAll();

    threadStart("consumer 1", 31, work, " A_");
    threadStart("consumer 2", 31, work, " B_");

    intrEnable();
    while (1) {}
}

void work(void *arg) {
    char *p = (char *) arg;
    // while (1) { consolePuts(p); }
    while (1) {
        consolePuts(p);
        char byte = ioqueueGetchar(&keyboardBuf);
        consolePutchar(byte);
    }
}
