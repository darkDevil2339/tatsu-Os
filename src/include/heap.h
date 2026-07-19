#ifndef HEAP_H
#define HEAP_H
#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include "../include/kstring.h"

struct memory_chunk {
    size_t size;
    struct memory_chunk* next;
    struct memory_chunk* prev;
    bool is_free;
};
void memory_init(void* heap_start, size_t total_size);
void* kmalloc(size_t size);
void kfree(void* ptr);
#endif // HEAP_H