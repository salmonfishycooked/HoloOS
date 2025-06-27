#ifndef __INCLUDE_STRING_H
#define __INCLUDE_STRING_H

#include <stdint.h>

    #ifndef NULL
        #define NULL 0
    #endif

void memset(void *dst, uint8 val, uint32 size);
void memcpy(void *dst, void *src, uint32 size);
int memcmp(const void *a, const void *b, uint32 size);
char *strcpy(char *dst, const char *src);
uint32 strlen(const char *str);
int8 strcmp(const char *str1, const char *str2);
char *strchr(const char *str, uint8 ch);
char *strrchr(const char *str, uint8 ch);
char *strcat(char *dst, const char *src);
uint32 strchrs(const char *str, uint8 ch);

#endif
