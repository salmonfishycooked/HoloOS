#include <stdint.h>
#include <string.h>
#include <kernel/global.h>
#include <kernel/thread.h>
#include <kernel/tss.h>
#include <kernel/print.h>

struct tss {
    uint32    backlink;
    uint32    *esp0;
    uint32    ss0;
    uint32    *esp1;
    uint32    ss1;
    uint32    *esp2;
    uint32    ss2;
    uint32    cr3;
    uint32    (*eip) (void);
    uint32    eflags;
    uint32    eax;
    uint32    ecx;
    uint32    edx;
    uint32    ebx;
    uint32    esp;
    uint32    ebp;
    uint32    esi;
    uint32    edi;
    uint32    es;
    uint32    cs;
    uint32    ss;
    uint32    ds;
    uint32    fs;
    uint32    gs;
    uint32    ldt;
    uint32    trace;
    uint32    ioBase;
};
static struct tss tss;

void updateTssEsp(struct taskStruct *pthread) {
    tss.esp0 = (uint32 *) ((uint32) pthread + PG_SIZE);
}

static struct gdtDesc makeGdtDesc(uint32 *descAddr, uint32 limit, uint8 attrLow, uint8 attrHigh) {
    uint32 descBase = (uint32) descAddr;

    struct gdtDesc desc;
    desc.limitLowWord = limit & 0x0000ffff;
    desc.baseLowWord = descBase & 0x0000ffff;
    desc.baseMidByte = (descBase &0x00ff0000) >> 16;
    desc.attrLowByte = attrLow;
    desc.limitAttrHighByte = (uint8) ((limit & 0x000f0000) >> 16) + attrHigh;
    desc.baseHighByte = descBase >> 24;

    return desc;
}

// tssInit creates TSS Descriptor in GDT and reloads GDT.
void tssInit() {
    puts("tss init starts...\n");

    uint32 tssSize = sizeof(tss);
    memset(&tss, 0, tssSize);

    tss.ss0 = SELECTOR_K_STACK;
    tss.ioBase = tssSize;

    // GDT is located in 0x900, and tss is the 4th (starts from 0).
    struct gdtDesc tssDesc = makeGdtDesc((uint32 *) &tss, tssSize - 1, TSS_ATTR_LOW, TSS_ATTR_HIGH);
    *((struct gdtDesc *) 0xc0000920) = tssDesc;

    // adds code and data descriptor that are dpl3,
    // in 5th and 6th position respectively.
    struct gdtDesc codeDesc = makeGdtDesc((uint32 *) 0, 0xfffff, GDT_CODE_ATTR_LOW_DPL3, GDT_ATTR_HIGH);
    *((struct gdtDesc *) 0xc0000928) = codeDesc;
    
    struct gdtDesc dataDesc = makeGdtDesc((uint32 *) 0, 0xfffff, GDT_DATA_ATTR_LOW_DPL3, GDT_ATTR_HIGH);
    *((struct gdtDesc *) 0xc0000930) = dataDesc;

    uint64 gdtOperand = (((uint64) 0xc0000900) << 16) | (8 * 7 - 1);
    asm volatile ("lgdt %0": :"m"(gdtOperand));
    asm volatile ("ltr %w0": :"r"(SELECTOR_TSS));

    puts("tss init done!\n");
}
