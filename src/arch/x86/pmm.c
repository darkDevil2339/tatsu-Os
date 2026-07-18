#include "../../include/pmm.h"
#include "../../include/kstring.h"

uint32_t* pmm_bitmap;
uint32_t  pmm_max_blocks;
uint32_t  pmm_bitmap_size; // in bytes


static inline void bitmap_set(uint32_t bit) {
    pmm_bitmap[bit >> 5] |= (1 << (bit & 31));
}

static inline void bitmap_clear(uint32_t bit) {
    pmm_bitmap[bit >> 5] &= ~(1 << (bit & 31));
}

static inline int bitmap_test(uint32_t bit) {
    return pmm_bitmap[bit >> 5] & (1 << (bit & 31));
}

void pmm_init(uint32_t mem_size, uint32_t bitmap_start) {
    pmm_max_blocks = mem_size / PMM_BLOCK_SIZE;
    pmm_bitmap_size = (pmm_max_blocks + 7) / 8;

    pmm_bitmap = (uint32_t*)bitmap_start;

    // Mark all memory as used
    memset(pmm_bitmap, 0xFF, pmm_bitmap_size);
}

void pmm_mark_free(uint32_t addr) {
    if (addr % PMM_BLOCK_SIZE)
        return;

    uint32_t bit = addr / PMM_BLOCK_SIZE;
    if (bit < pmm_max_blocks)
        bitmap_clear(bit);
}

void pmm_mark_used(uint32_t addr) {
    if (addr % PMM_BLOCK_SIZE)
        return;

    uint32_t bit = addr / PMM_BLOCK_SIZE;
    if (bit < pmm_max_blocks)
        bitmap_set(bit);
}

void* pmm_alloc_block(void) {
    uint32_t total_words = (pmm_max_blocks + 31) / 32;

    for (uint32_t i = 0; i < total_words; i++) {
        if (pmm_bitmap[i] != 0xFFFFFFFF) {
            for (uint32_t j = 0; j < 32; j++) {
                uint32_t bit = i * 32 + j;

                if (bit >= pmm_max_blocks)
                    return 0;

                if (!bitmap_test(bit)) {
                    bitmap_set(bit);
                    return (void*)(bit * PMM_BLOCK_SIZE);
                }
            }
        }
    }
    return 0; // Out of memory
}

void pmm_free_block(void* addr) {
    pmm_mark_free((uint32_t)addr);
}
