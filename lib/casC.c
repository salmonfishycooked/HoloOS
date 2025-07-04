#include <cas.h>
#include <kernel/print.h>

bool compareAndSwap32(int *addr, int expected, int newVal) {
    return CAS32(addr, expected, newVal) == expected;
}
