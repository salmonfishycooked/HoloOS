#ifndef __INCLUDE_KERNEL_ASSERT_H
#define __INCLUDE_KERNEL_ASSERT_H

void panicSpin(char *filename, int line, const char *func, const char *condition);

#define PANIC(...)  panicSpin(__FILE__, __LINE__, __func__, __VA_ARGS__)

    #ifndef NDEBUG
        #define ASSERT(CONDITION)  ((void) 0)
    #else
        #define ASSERT(CONDITION)  if (!CONDITION) { PANIC(#CONDITION); }
    #endif

#endif
