#ifndef KSTRING_H
#define KSTRING_H

#include <stdint.h>
#include <stddef.h>

void* memset(void* dest, int val, size_t len);
void* memcpy(void* dest, const void* src, size_t len);
void* memcmp(const void* s1, const void* s2, size_t n);


#endif