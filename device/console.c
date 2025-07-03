#include <stdint.h>
#include <kernel/sync.h>
#include <kernel/print.h>

static struct lock consoleLock;

void consoleInit() {
    lockInit(&consoleLock);
}

static void consoleAcquire() {
    lockAcquire(&consoleLock);
}

static void consoleRelease() {
    lockRelease(&consoleLock);
}

void consolePuts(char *str) {
    consoleAcquire();
    puts(str);
    consoleRelease();
}

void consolePutchar(uint8 ch) {
    consoleAcquire();
    putchar(ch);
    consoleRelease();
}

void consolePutint(uint32 num) {
    consoleAcquire();
    putint(num);
    consoleRelease();
}
