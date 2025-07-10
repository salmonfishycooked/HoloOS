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
#include <kernel/syscall.h>

int pidProgA = 0;
int pidProgB = 0;

void work1(void *arg);
void work2(void *arg);
void uWork1(void *arg);
void uWork2(void *arg);

int main() {
    clearScreen();
    puts("HoloOS is booting...\n");

    initAll();

    processExecute(uWork1, "user program A");
    processExecute(uWork2, "user program B");
    threadStart("thread 1", 31, work1, "A");
    threadStart("thread 2", 31, work2, "B");

    intrEnable();
    while (1) {}
}

void work1(void *arg) {
    char *p = (char *) arg;
    while (1) {
        consolePuts("pid of thread 1: ");
        consolePutint(getpid());
        consolePutchar('\n');
        consolePuts("pid of user program A: ");
        consolePutint(pidProgA);
        consolePutchar('\n');
    }
}

void work2(void *arg) {
    char *p = (char *) arg;
    while (1) {
        consolePuts("pid of thread 2: ");
        consolePutint(getpid());
        consolePutchar('\n');
        consolePuts("pid of user program B: ");
        consolePutint(pidProgB);
        consolePutchar('\n');
    }
}

void uWork1(void *arg) {
    char *p = (char *) arg;
    pidProgA = getpid();
    while (1) {}
}

void uWork2(void *arg) {
    char *p = (char *) arg;
    pidProgB = getpid();
    while (1) {}
}
