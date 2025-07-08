#include <kernel/print.h>
#include <kernel/init.h>
#include <kernel/debug.h>
#include <kernel/memory.h>
#include <kernel/thread.h>
#include <kernel/interrupt.h>
#include <device/console.h>
#include <device/keyboard.h>
#include <device/ioqueue.h>
#include <kernel/process.h>
#include <cas.h>

int valueA = 0;
int valueB = 0;

void work1(void *arg);
void work2(void *arg);
void uWork1(void *arg);
void uWork2(void *arg);

int main() {
    clearScreen();
    puts("HoloOS is booting...\n");

    initAll();

    threadStart("consumer 1", 31, work1, " A_");
    threadStart("consumer 2", 31, work2, " B_");
    processExecute(uWork1, "user program A");
    processExecute(uWork2, "user program B");

    intrEnable();
    while (1) {}
}

void work1(void *arg) {
    char *p = (char *) arg;
    while (1) {
        consolePuts("valueA: 0x");
        consolePutint(valueA);
        consolePutchar('\n');
    }
}

void work2(void *arg) {
    char *p = (char *) arg;
    while (1) {
        consolePuts("valueB: 0x");
        consolePutint(valueB);
        consolePutchar('\n');
    }
}

void uWork1(void *arg) {
    char *p = (char *) arg;
    while (1) {
        valueA += 1;
    }
}

void uWork2(void *arg) {
    char *p = (char *) arg;
    while (1) {
        valueB += 1;
    }
}
