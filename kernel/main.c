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
#include <stdio.h>


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
    // threadStart("thread 1", 31, work1, "A");
    // threadStart("thread 2", 31, work2, "B");

    intrEnable();
    while (1) {}
}


void work1(void *arg) {
    char *p = (char *) arg;
    consolePuts("pid of thread 1: ");
    consolePutint(getpid());
    consolePutchar('\n');
    while (1) {}
}


void work2(void *arg) {
    char *p = (char *) arg;
    consolePuts("pid of thread 2: ");
    consolePutint(getpid());
    consolePutchar('\n');
    while (1) {}
}


void uWork1(void *arg) {
    char *p = "holo";
    printf("%s: What's up! my pid is %d%c", p, getpid(), '\n');
    while (1) {}
}


void uWork2(void *arg) {
    char *p = "myuri";
    printf("%s: What's up! my pid is %d%c", p, getpid(), '\n');
    while (1) {}
}
