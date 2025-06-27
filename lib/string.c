#include <string.h>
#include <kernel/global.h>
#include <kernel/debug.h>

// memset will set size bytes starting from dst to val.
void memset(void *dst, uint8 val, uint32 size) {
    ASSERT(dst != NULL);

    uint8 *dstC = (uint8 *) dst;
    for (; size; --size) {
        *dstC = val;
        ++dstC;
    }
}

// memcpy will copy size bytes of src into dst.
void memcpy(void *dst, void *src, uint32 size) {
    ASSERT(dst != NULL && src != NULL);

    uint8 *dstC = dst, *srcC = src;
    for (; size; --size) {
        *dstC = *srcC;
        ++dstC; ++srcC;
    }
}

// memcmp will compare a with b.
// return 0 if a == b.
// return 1 if a > b.
// return -1 if a < b. 
int memcmp(const void *a, const void *b, uint32 size) {
    ASSERT(a != NULL && b != NULL);

    const uint8 *ac = (const uint8 *) a;
    const uint8 *bc = (const uint8 *) b;
    for (; size; --size) {
        if (*ac > *bc) { return 1; }
        if (*ac < *bc) { return -1; }
        ++ac; ++bc;
    }

    return 0;
}

// strcpy will copy src to dst.
char *strcpy(char *dst, const char *src) {
    ASSERT(dst != NULL && src != NULL);

    char *ret = dst;
    for (; *src; ++src, ++dst) { *dst = *src; }

    return ret;
}

// strlen will return the length of str.
uint32 strlen(const char *str) {
    ASSERT(str != NULL);

    const char *start = str;
    for (; *str; ++str) {}

    return str - start;
}

// strcmp will compare str1 with str2.
// return 0 if str1 == str2.
// return 1 if str1 > str2.
// return -1 if str1 < str2.
int8 strcmp(const char *str1, const char *str2) {
    ASSERT(str1 != NULL && str2 != NULL);

    while (*str1 != 0 && *str1 == *str2) {
        ++str1; ++str2;
    }

    if (*str1 == 0 && *str2 == 0) { return 0; }

    return *str1 > *str2 ? 1 : -1;
}

// strchr will return the address of ch in str that appears at left-most of str.
// return NULL if ch doesn't exist in str.
char *strchr(const char *str, uint8 ch) {
    ASSERT(str != NULL);

    for (; *str; ++str) {
        if (*str == ch) { return (char *) str; }
    }

    return NULL;
}

// strrchr will return the address of ch in str that appears at right-most of str.
// return NULL if ch doesn't exist in str.
char *strrchr(const char *str, uint8 ch) {
    ASSERT(str != NULL);

    const char *lastChar = NULL;
    for (; *str; ++str) {
        if (*str == ch) { lastChar = str; }
    }

    return (char *) lastChar;
}

// strcat will concatenate src after dst
char *strcat(char *dst, const char *src) {
    ASSERT(dst != NULL && src != NULL);

    char *start;
    while (*dst) { ++dst; }
    for (; *src; ++src, ++dst) { *dst = *src; }
    *dst = '\0';

    return start;
}

// strchrs will return the count of ch that appears in str.
uint32 strchrs(const char *str, uint8 ch) {
    ASSERT(str != NULL);

    uint32 cnt = 0;
    for (; *str; ++str) { cnt += (*str == ch); }

    return cnt;
}
