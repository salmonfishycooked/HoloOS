#include <kernel/print.h>
#include "stdint.h"

// putint will output a unsigned integer onto screen (hex mode)
void putint(uint32 num) {
    uint8 digit[8];

    for (int i = 0; i < 8; i++) {
        int shr = (7 - i) * 4;
        digit[i] = (num >> shr) & 0xF;
    }

    bool leading = 1;
    for (int i = 0; i < 8; i++) {
        if (leading && digit[i] == 0) { continue; }

        leading = 0;
        char ch = digit[i] + 48;
        if (digit[i] >= 10) { ch = digit[i] + 87; }

        putchar(ch);
    }
}
