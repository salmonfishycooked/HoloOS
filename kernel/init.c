#include <kernel/print.h>
#include <kernel/interrupt.h>

void initAll() {
    puts("inn\n");
    idtInit();                  // initialize interrupt
}
