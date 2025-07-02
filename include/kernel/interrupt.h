#ifndef __INCLUDE_KERNEL_INTERRUPT_H
#define __INCLUDE_KERNEL_INTERRUPT_H

#include <stdint.h>

typedef void *intrHandler;

enum intrStatus {INTR_OFF, INTR_ON};

void idtInit();

enum intrStatus intrGetStatus();
enum intrStatus intrSetStatus(enum intrStatus status);
enum intrStatus intrEnable();
enum intrStatus intrDisable();

void registerHandler(uint8 intrNum, intrHandler func);

#endif
