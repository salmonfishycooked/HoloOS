#include <kernel/print.h>
#include <kernel/interrupt.h>
#include <kernel/init.h>
#include <kernel/memory.h>
#include <device/timer.h>
#include <kernel/thread.h>
#include <device/console.h>

void initAll() {
    puts("init all...\n");

    idtInit();                  // initialize interrupt
    timerInit();                // initialize PIT8253 for timer interrupt
    memInit();                  // initialize memory

    threadSupportInit();        // initialize thread support
    consoleInit();              // initialize terminal device
}
