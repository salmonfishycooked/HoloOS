#include <kernel/thread.h>
#include <kernel/debug.h>
#include <kernel/memory.h>
#include <kernel/process.h>
#include <kernel/sync.h>
#include <kernel/global.h>
#include <string.h>
#include <device/console.h>
#include <kernel/debug.h>
#include <kernel/tss.h>
#include <kernel/print.h>
#include <stdio.h>

extern struct spinlock threadReadyListLock;
extern struct spinlock threadAllListLock;
extern struct list     threadReadyList;
extern struct list     threadAllList;


extern void intrExit();


// startProcess builds initial context of a new user process.
static void startProcess(void *filename) {
    void *func = filename;

    struct taskStruct *cur = threadCurrent();
    cur->selfKStack += sizeof(struct threadStack);
    struct intrStack *procStack = (struct intrStack *) cur->selfKStack;

    procStack->edi = 0;
    procStack->esi = 0;
    procStack->ebp = 0;
    procStack->espDummy = 0;
    procStack->ebx = 0;
    procStack->edx = 0;
    procStack->ecx = 0;
    procStack->eax = 0;
    procStack->gs = 0;
    procStack->ds = SELECTOR_U_DATA;
    procStack->es = SELECTOR_U_DATA;
    procStack->fs = SELECTOR_U_DATA;
    procStack->eip = func;
    procStack->cs = SELECTOR_U_CODE;
    procStack->eflags = (EFLAGS_IOPL_0 | EFLAGS_MBS | EFLAGS_IF_1);
    procStack->esp = (void *) ((uint32) getPage(PF_USER, USER_STACK3_VADDR) + PG_SIZE);
    procStack->ss = SELECTOR_U_DATA;

    // ignore vecNo
    procStack = (struct intrStack *) ((uint32) procStack + 4);

    asm volatile ("movl %0, %%esp; jmp intrExit" : : "g"(procStack) : "memory");
}


// createPageDir allocates a page frame for new pageDir,
// and copys kernel pde to new pageDir.
static uint32 *createPageDir() {
    uint32 *pageDirVaddr = getKernelPages(1);
    if (pageDirVaddr == NULL) {
        consolePuts("create page dir: get kernel page failed!\n");
        return NULL;
    }

    // copy kernel pde
    memcpy((uint32 *) ((uint32) pageDirVaddr + 0x300 * 4),
           (uint32 *) (0xfffff000 + 0x300 * 4),
           1020);

    // page dir self mapping
    uint32 newPageDirPhyAddr = v2p((uint32) pageDirVaddr);
    pageDirVaddr[1023] = (newPageDirPhyAddr | PG_US_U | PG_RW_W | PG_P_1);

    return pageDirVaddr;
}


// createUserVaddrBitmap creates virtual address bitmap of the user process.
static void createUserVaddrBitmap(struct taskStruct *userProg) {
    userProg->userVaddr.vaddrStart = USER_VADDR_START;
    uint32 bitmapPgCnt = ((0xc0000000 - USER_VADDR_START) / PG_SIZE / 8 + PG_SIZE - 1) / PG_SIZE;
    userProg->userVaddr.vaddrBitmap.bits = getKernelPages(bitmapPgCnt);
    userProg->userVaddr.vaddrBitmap.size = (0xc0000000 - USER_VADDR_START) / PG_SIZE / 8;
    bitmapInit(&userProg->userVaddr.vaddrBitmap);
}


// processExecute creates user process, and executes it.
void processExecute(void *filename, char *name) {
    // PCB of user process is maintained by kernel.
    struct taskStruct *thread = getKernelPages(1);
    threadInit(thread, name, DEFAULT_PRIO);
    createUserVaddrBitmap(thread);
    threadCreate(thread, startProcess, filename);
    thread->pageDir = createPageDir();

    enum intrStatus oldStatus = intrDisable();

    spinlockAcquire(&threadReadyListLock);
    ASSERT(!listExist(&threadReadyList, &thread->generalTag));
    listAppend(&threadReadyList, &thread->generalTag);
    spinlockRelease(&threadReadyListLock);

    spinlockAcquire(&threadAllListLock);
    ASSERT(!listExist(&threadAllList, &thread->allListTag));
    listAppend(&threadAllList, &thread->allListTag);
    spinlockRelease(&threadAllListLock);

    intrSetStatus(oldStatus);
}


// pageDirActivate changes to corresponding page directory.
static void pageDirActivate(struct taskStruct *pthread) {
    // pageDirPhyAddr is decided by kernel thread or user process
    uint32 pageDirPhyAddr = 0x100000;
    if (pthread->pageDir != NULL) {
        pageDirPhyAddr = v2p((uint32) pthread->pageDir);
    }

    asm volatile ("movl %0, %%cr3" : : "r"(pageDirPhyAddr) : "memory");
}


// processActivate changes to corresponding page directory,
// and updates esp0 of tss if pthread is user process.
void processActivate(struct taskStruct *pthread) {
    ASSERT(pthread != NULL);

    pageDirActivate(pthread);

    // if it is user process, we need to update esp0 of tss.
    if (pthread->pageDir) {
        updateTssEsp(pthread);
    }
}
