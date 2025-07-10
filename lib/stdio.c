#include <stdio.h>
#include <string.h>
#include <kernel/syscall.h>


#define va_start(ap, v)     ap = (va_list) &v
#define va_arg(ap, t)       *((t *) (ap += 4))
#define va_end(ap)          ap = NULL


typedef void *              va_list;


// itoa converts 10-base v to string in base passed in.
// stores at the position pointed by *bufp.
static void itoa(uint32 v, char **bufp, uint8 base) {
    uint32 digit = v % base;
    v = v / base;
    if (v) { itoa(v, bufp, base); }

    if (digit < 10) {
        *((*bufp)++) = digit + '0';
    } else {
        *((*bufp)++) = digit - 10 + 'A';
    }
}


// vsprintf formats format to str.
static uint32 vsprintf(char *str, const char *format, va_list ap) {
    char *buf = str;
    while (*format) {
        if (*format != '%') {
            *buf = *format;
            ++buf; ++format;
            continue;
        }

        char ch = *(++format);
        switch (ch) {
            case 'x': {
                int32 argInt = va_arg(ap, int);
                itoa(argInt, &buf, 16);
                ++format;
                break;
            }

            case 'd': {
                int32 argInt = va_arg(ap, int);
                if (argInt < 0) {
                    argInt = -argInt;
                    *buf = '-';
                    ++buf;
                }
                itoa(argInt, &buf, 10);
                ++format;
                break;
            }

            case 'c': {
                *buf = va_arg(ap, char);
                ++buf; ++format;
                break;
            }

            case 's': {
                char *argStr = va_arg(ap, char *);
                strcpy(buf, argStr);
                buf += strlen(argStr);
                ++format;
                break;
            }
        }
    }

    *buf = '\0';

    return buf - str;
}


uint32 printf(const char *format, ...) {
    va_list args;
    va_start(args, format);

    char buf[1024] = {0};
    vsprintf(buf, format, args);

    va_end(args);

    return write(buf);
}

uint32 sprintf(char *buf, const char *format, ...) {
    va_list args;
    va_start(args, format);

    uint32 ret = vsprintf(buf, format, args);

    va_end(args);

    return ret;
}
