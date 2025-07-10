#include <kernel/print.h>
#include <kernel/interrupt.h>
#include <kernel/init.h>
#include <kernel/memory.h>
#include <device/timer.h>
#include <kernel/thread.h>
#include <device/console.h>
#include <device/keyboard.h>
#include <kernel/tss.h>
#include <kernel/syscall.h>

void initAll() {
    puts("init all...\n");

    idtInit();                  // initialize interrupt
    memInit();                  // initialize memory
    threadSupportInit();        // initialize thread support
    consoleInit();              // initialize terminal device
    keyboardInit();             // initialize keyboard device
    tssInit();                  // initialize tss
    syscallInit();              // initialize syscall for user process

    timerInit();                // initialize PIT8253 for timer interrupt
}
