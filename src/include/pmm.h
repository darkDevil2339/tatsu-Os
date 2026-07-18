#ifndef PMM_H
#define PMM_H

#include <stdint.h>

#define PMM_BLOCK_SIZE 4096
extern uint32_t* pmm_bitmap;
extern uint32_t  pmm_bitmap_size;
extern uint32_t  pmm_max_blocks;

void pmm_init(uint32_t mem_size, uint32_t bitmap_start_addr);
void pmm_mark_used(uint32_t addr);
void pmm_mark_free(uint32_t addr);
void* pmm_alloc_block();
void pmm_free_block(void* addr);

#endif