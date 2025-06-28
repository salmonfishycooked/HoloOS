#ifndef __INCLUDE_KERNEL_BITMAP_H
#define __INCLUDE_KERNEL_BITMAP_H

#include <stdint.h>

struct bitmap {
    uint8 *bits;        // points to the start address of the bitmap
    uint32 size;        // the size of bitmap (unit: byte)
};

void bitmapInit(struct bitmap *bmap);
bool bitmapTest(struct bitmap *bmap, uint32 bitIdx);
int bitmapScan(struct bitmap *bmap, uint32 cnt);
void bitmapSet(struct bitmap *bmap, uint32 bitIdx, int8 val);

#endif
