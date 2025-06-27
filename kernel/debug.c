#include <kernel/debug.h>
#include <kernel/print.h>
#include <kernel/interrupt.h>

void panicSpin(char *filename, int line, const char *func, const char *condition) {
    intrDisable();

    puts("\n\n\n!!! ERROR !!!\n");
    puts("filename: ");  puts(filename);           putchar('\n');
    puts("line: 0x");    putint(line);             putchar('\n');
    puts("function: ");  puts((char *) func);      putchar('\n');
    puts("condition: "); puts((char *) condition); putchar('\n');

    while (1) {}
}
