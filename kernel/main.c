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
    threadStart("thread 1", 31, work1, "A");
    threadStart("thread 2", 31, work2, "B");

    intrEnable();
    while (1) {}
}


void work1(void *arg) {
    char *p = (char *) arg;
    void *addr1 = sysMalloc(256);
    void *addr2 = sysMalloc(255);
    void *addr3 = sysMalloc(254);
    consolePuts("thread 1 malloc addr: ");
    consolePutint((uint32) addr1); consolePuts(", ");
    consolePutint((uint32) addr2); consolePuts(", ");
    consolePutint((uint32) addr3);
    consolePutchar('\n');

    int cpuDelay = 100000;
    while (cpuDelay--) {}

    // sysFree(addr1);
    // sysFree(addr2);
    // sysFree(addr3);

    while (1) {}
}


void work2(void *arg) {
    char *p = (char *) arg;
    void *addr1 = sysMalloc(256);
    void *addr2 = sysMalloc(255);
    void *addr3 = sysMalloc(254);
    consolePuts("thread 2 malloc addr: ");
    consolePutint((uint32) addr1); consolePuts(", ");
    consolePutint((uint32) addr2); consolePuts(", ");
    consolePutint((uint32) addr3);
    consolePutchar('\n');

    int cpuDelay = 100000;
    while (cpuDelay--) {}

    // sysFree(addr1);
    // sysFree(addr2);
    // sysFree(addr3);

    while (1) {}
}


void uWork1(void *arg) {
    char *p = (char *) arg;
    void *addr1 = malloc(256);
    void *addr2 = malloc(255);
    void *addr3 = malloc(254);
    printf("program a malloc addr: 0x%x, 0x%x, 0x%x\n", (uint32) addr1, (uint32) addr2, (uint32) addr3);

    int cpuDelay = 100000;
    while (cpuDelay--) {}

    free(addr1);
    free(addr2);
    free(addr3);

    while (1) {}
}


void uWork2(void *arg) {
    char *p = (char *) arg;
    void *addr1 = malloc(256);
    void *addr2 = malloc(255);
    void *addr3 = malloc(254);
    printf("program b malloc addr: 0x%x, 0x%x, 0x%x\n", (uint32) addr1, (uint32) addr2, (uint32) addr3);

    int cpuDelay = 100000;
    while (cpuDelay--) {}

    free(addr1);
    free(addr2);
    free(addr3);

    while (1) {}
}
