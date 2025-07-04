#ifndef __INCLUDE_CAS_H
#define __INCLUDE_CAS_H

int CAS32(int *addr, int expected, int newVal);
bool compareAndSwap32(int *addr, int expected, int newVal);

#endif
