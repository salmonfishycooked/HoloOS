#include <kernel/interrupt.h>
#include <kernel/global.h>
#include <stdint.h>
#include <kernel/io.h>
#include <kernel/print.h>

#define IDT_DESC_CNT 0x81       // number of supported interrupts currently

#define PIC_M_CTRL   0x20       // PIC Master's control port
#define PIC_M_DATA   0x21       // PIC Master's data port
#define PIC_S_CTRL   0xa0       // PIC Slave's control port
#define PIC_S_DATA   0xa1       // PIC Slave's data port

#define EFLAGS_IF    0x00000200  // IF flag's position in eflags
#define GET_EFLAGS(EFLAG_VAR)   asm volatile ("pushfl; popl %0" : "=g"(EFLAG_VAR))

// gateDesc is descriptor of interrupt gate
struct gateDesc {
    uint16 funcOffsetLowWord;
    uint16 selector;
    uint8  dCount;
    uint8  attribute;
    uint16 funcOffsetHighWord;
};

static void makeIdtDesc(struct gateDesc *desc, uint8 attr, intrHandler func);

// idt is interrupt descriptor table
static struct gateDesc idt[IDT_DESC_CNT];

char *intrName[IDT_DESC_CNT];
intrHandler idtTable[IDT_DESC_CNT];

// intrEntryTable is defined in kernel.S
// records the entry address of every interrupt
extern intrHandler intrEntryTable[IDT_DESC_CNT];

extern void syscallHandler();

static void generalIntrHandler(uint8 num) {
    // IRQ7 and IRQ15 emit spurious interrupt,
    // and we don't need to handle them.
    if (num == 0x27 || num == 0x2f) { return; }

    puts("\n\n");
    puts("!!!!!     exception message     !!!!!\n");
    puts(intrName[num]);
    putchar('\n');

    if (num == 14) {
        uint32 pageFaultVaddr = 0;
        asm volatile ("movl %%cr2, %0" : "=r"(pageFaultVaddr));
        puts("page fault address is: ");
        putint(pageFaultVaddr);
        putchar('\n');
    }

    puts("!!!!!     exception message end     !!!!!\n");
    while (1) {}
}

static void exceptionInit() {
    for (int i = 0; i < IDT_DESC_CNT; i++) {
        idtTable[i] = generalIntrHandler;
        intrName[i] = "unknown";
    }

   intrName[0] = "#DE Divide Error";
   intrName[1] = "#DB Debug Exception";
   intrName[2] = "NMI Interrupt";
   intrName[3] = "#BP Breakpoint Exception";
   intrName[4] = "#OF Overflow Exception";
   intrName[5] = "#BR BOUND Range Exceeded Exception";
   intrName[6] = "#UD Invalid Opcode Exception";
   intrName[7] = "#NM Device Not Available Exception";
   intrName[8] = "#DF Double Fault Exception";
   intrName[9] = "Coprocessor Segment Overrun";
   intrName[10] = "#TS Invalid TSS Exception";
   intrName[11] = "#NP Segment Not Present";
   intrName[12] = "#SS Stack Fault Exception";
   intrName[13] = "#GP General Protection Exception";
   intrName[14] = "#PF Page-Fault Exception";
   // intrName[15] is reserved by intel
   intrName[16] = "#MF x87 FPU Floating-Point Error";
   intrName[17] = "#AC Alignment Check Exception";
   intrName[18] = "#MC Machine-Check Exception";
   intrName[19] = "#XF SIMD Floating-Point Exception";
}

// picInit initializes Programmable Interrupt Controller
static void picInit() {
    // initialize master
    outb(PIC_M_CTRL, 0x11);         // ICW1: edge triggered mode, cascade 8259, open ICW4
    outb(PIC_M_DATA, 0x20);         // ICW2: set start number of interrupt is 0x20 (namely 0x20 ~ 0x27)
    outb(PIC_M_DATA, 0x04);         // ICW3: set IR2 connected to slave
    outb(PIC_M_DATA, 0x01);         // ICW4: 8086 mode, manual EOI

    // initialize slave
    outb(PIC_S_CTRL, 0x11);         // ICW1: edge triggered mode, cascade 8259, open ICW4
    outb(PIC_S_DATA, 0x28);         // ICW2: set start number of interrupt is 0x28 (namely 0x28 ~ 0x2f)
    outb(PIC_S_DATA, 0x02);         // ICW3: set the slave connected to IR2 on master
    outb(PIC_S_DATA, 0x01);         // ICW4: 8086 mode, manual EOI

    // only open IR0 on master. (Clock Interrupt)
    //           IR1 on master. (Keyboard Interrupt)
    // masks all other's interrupt.
    outb(PIC_M_DATA, 0xfc);
    outb(PIC_S_DATA, 0xff);

    puts("pic init done.\n");
}

// makeIdtDesc will make the desc you passed in an effective desc
// according to the arguments
static void makeIdtDesc(struct gateDesc *desc, uint8 attr, intrHandler func) {
    desc->funcOffsetLowWord = (uint32) func & 0x0000FFFF;
    desc->selector = SELECTOR_K_CODE;
    desc->dCount = 0;
    desc->attribute = attr;
    desc->funcOffsetHighWord = ((uint32) func & 0xFFFF0000) >> 16;
}

static void idtDescInit() {
    for (int i = 0; i < IDT_DESC_CNT; i++) {
        makeIdtDesc(&idt[i], IDT_DESC_ATTR_DPL0, intrEntryTable[i]);
    }

    // handle syscall.
    makeIdtDesc(&idt[0x80], IDT_DESC_ATTR_DPL3, syscallHandler);

    puts("idtDescInit done.\n");
}

// intrEnable will turn on interrupt
enum intrStatus intrEnable() {
    enum intrStatus oldStatus = INTR_ON;
    if (intrGetStatus() == INTR_OFF) {
        asm volatile ("sti");
        oldStatus = INTR_OFF;
    }

    return oldStatus;
}

// intrDisable will turn off interrupt
enum intrStatus intrDisable() {
    enum intrStatus oldStatus = INTR_OFF;
    if (intrGetStatus() == INTR_ON) {
        asm volatile ("cli");
        oldStatus = INTR_ON;
    }

    return oldStatus;
}

// intrGetStatus will return current state of interrupt (on/off)
enum intrStatus intrGetStatus() {
    uint32 eflags = 0;
    GET_EFLAGS(eflags);

    return eflags & EFLAGS_IF ? INTR_ON : INTR_OFF;
}

// intrSetStatus sets the current state of interrupt. (on/off)
enum intrStatus intrSetStatus(enum intrStatus status) {
    enum intrStatus oldStatus = intrGetStatus();
    switch(status) {
        case INTR_OFF: intrDisable(); break;
        case INTR_ON: intrEnable(); break;
    }

    return oldStatus;
}

// registerHandler reigsters handler of interrupt number to be func.
void registerHandler(uint8 intrNum, intrHandler func) {
    idtTable[intrNum] = func;
}

// idtInit will do all of the work of interrupt initialization
void idtInit() {
    puts("idtInit is starting...\n");

    idtDescInit();
    exceptionInit();
    picInit();

    // load idt
    uint64 idtOperand = ((sizeof(idt) - 1) | (((uint64) (uint32) idt) << 16));
    asm volatile("lidt %0" : : "m"(idtOperand));

    puts("idtInit done.\n");
}
