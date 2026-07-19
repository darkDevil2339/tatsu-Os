#include "../include/heap.h"


struct memory_chunk* first_chunk;

void memory_init(void* heap_start, size_t total_size) {
    first_chunk = (struct memory_chunk*)heap_start;
    first_chunk->size = total_size - sizeof(struct memory_chunk);
    first_chunk->next = NULL;
    first_chunk->prev = NULL;
    first_chunk->is_free = true;
}

void* kmalloc(size_t size) {
    struct memory_chunk* current = first_chunk;

    while (current != NULL) {
        if (current->is_free && current->size >= size) {
            // Split the chunk if it's larger than needed
            if (current->size > size + sizeof(struct memory_chunk)) {
                struct memory_chunk* new_chunk = (struct memory_chunk*)((char*)current + sizeof(struct memory_chunk) + size);
                new_chunk->size = current->size - size - sizeof(struct memory_chunk);
                new_chunk->next = current->next;
                new_chunk->prev = current;
                new_chunk->is_free = true;

                if (current->next != NULL) {
                    current->next->prev = new_chunk;
                }

                current->next = new_chunk;
                current->size = size;
            }

            current->is_free = false;
            return (void*)((char*)current + sizeof(struct memory_chunk));
        }
        current = current->next;
    }

    return NULL; // No suitable chunk found

}

void kfree(void* ptr) {
    // Implementation for freeing memory
    struct memory_chunk* chunk = (struct memory_chunk*)((char*)ptr - sizeof(struct memory_chunk));
    chunk->is_free = true;
    if (chunk->prev != NULL && chunk->prev->is_free) {
        // Merge with previous chunk
        chunk->prev->size += sizeof(struct memory_chunk) + chunk->size;
        chunk->prev->next = chunk->next;
        if (chunk->next != NULL) {
            chunk->next->prev = chunk->prev;
        }
        chunk = chunk->prev; // Update current chunk to the merged one
    }
    if (chunk->next != NULL && chunk->next->is_free) {
        // Merge with next chunk
        chunk->size += sizeof(struct memory_chunk) + chunk->next->size;
        chunk->next = chunk->next->next;
        if (chunk->next != NULL) {
            chunk->next->prev = chunk;
        }
    }
}