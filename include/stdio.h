#ifndef _STDIO_H
#define _STDIO_H

//#include <unistd.h>
#include <defs.h>

int printf(const char *format, ...);
int scanf(char *buffer);

void* malloc(uint32_t size);
int getpid();

#endif
