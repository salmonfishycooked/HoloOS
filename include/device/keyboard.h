#ifndef __INCLUDE_DEVICE_KEYBOARD_H
#define __INCLUDE_DEVICE_KEYBOARD_H

extern struct ioqueue keyboardBuf;

void keyboardInit();

char ioqueueGetchar(struct ioqueue *ioq);
void ioqueuePutchar(struct ioqueue *ioq, char ch);

#endif
