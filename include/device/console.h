#ifndef __INCLUDE_DEVICE_CONSOLE_H
#define __INCLUDE_DEVICE_CONSOLE_H

#include <stdint.h>

void consoleInit();

void consolePuts(char *str);
void consolePutchar(uint8 ch);
void consolePutint(uint32 num);

#endif
