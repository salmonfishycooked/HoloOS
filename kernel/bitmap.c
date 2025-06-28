#include <kernel/bitmap.h>
#include <kernel/debug.h>
#include <kernel/print.h>
#include <string.h>

// bitmapInit initializes the bmap passed in.
// by set all of bits to 0.
void bitmapInit(struct bitmap *bmap) {
    memset(bmap->bits, 0, bmap->size);
}

// bitmapTest returns value of the bit at position bitIdx.
bool bitmapTest(struct bitmap *bmap, uint32 bitIdx) {
    return bmap->bits[bitIdx / 8] & (1 << (bitIdx % 8));
}

// bitmapScan finds the start index of cnt consecutive available bits.
// return > 0 if available enough
// return -1 if no enough bits
int bitmapScan(struct bitmap *bmap, uint32 cnt) {
    // find 0 bit byte by byte.
    int idx = 0, sz = bmap->size;
    while (idx < sz && bmap->bits[idx] == 0xff) { idx += 1; }

    // no enough bit can be allocated.
    if (idx == sz) { return -1; }

    // find cnt consecutive 0 bits.
    idx = idx * 8, sz = sz * 8;
    uint32 curCnt = 0;
    for (; idx < sz; idx++) {
        if (!bitmapTest(bmap, idx)) { curCnt += 1; }
        else { curCnt = 0; }

        if (curCnt == cnt) { return idx + 1 - cnt; }
    }

    return -1;
}

// bitmapSet sets the value at position bitIdx to val.
void bitmapSet(struct bitmap *bmap, uint32 bitIdx, int8 val) {
    ASSERT(val == 0 || val == 1);

    uint32 byteIdx = bitIdx / 8;
    uint32 bitPos = bitIdx % 8;
    if (val) { bmap->bits[byteIdx] |= (1 << bitPos); }
    else { bmap->bits[byteIdx] &= ~(1 << bitPos); }
}
