#ifndef __INCLUDE_KERNEL_TSS_H
#define __INCLUDE_KERNEL_TSS_H

void tssInit();

void updateTssEsp(struct taskStruct *pthread);

#endif
