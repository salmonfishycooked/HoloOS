#ifndef __INCLUDE_KERNEL_PROCESS_H
#define __INCLUDE_KERNEL_PROCESS_H

#define USER_VADDR_START    0x8048000
#define USER_STACK3_VADDR   (0xc0000000 - 0x1000)

#define DEFAULT_PRIO        16

void processActivate(struct taskStruct *pthread);
void processExecute(void *filename, char *name);

#endif
