#ifndef __INCLUDE_KERNEL_INTERRUPT_H
#define __INCLUDE_KERNEL_INTERRUPT_H

typedef void *intrHandler;

enum intrStatus {INTR_OFF, INTR_ON};

void idtInit();

enum intrStatus intrGetStatus();
enum intrStatus intrEnable();
enum intrStatus intrDisable();

#endif
