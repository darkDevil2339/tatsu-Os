#include "../../include/paging.h"
#include "../../include/framebuffer.h"

#define PAGE_PRESENT 0x1
#define PAGE_WRITE   0x2

// Keep this global so vmm.c can access it during early boot
__attribute__((aligned(4096)))
uint32_t page_directory[1024];

__attribute__((aligned(4096)))
uint32_t first_page_table[1024];

void paging_init(void) {
    // 1. Clear directory
    for (int i = 0; i < 1024; i++)
        page_directory[i] = 0x00000002;

    // 2. IDENTITY MAP the first 4MB (0x0 to 0x400000)
    for (uint32_t i = 0; i < 1024; i++) {
        first_page_table[i] = (i * 0x1000) | PAGE_PRESENT | PAGE_WRITE;
    }
    page_directory[0] = ((uint32_t)first_page_table) | PAGE_PRESENT | PAGE_WRITE;

    // 2.5 RECURSIVE MAPPING
    // Map the last entry of the PD to the PD itself.
    // This creates a virtual window at 0xFFC00000 to manage tables.
    page_directory[1023] = ((uint32_t)page_directory) | PAGE_PRESENT | PAGE_WRITE;

    // 3. Map Framebuffer (if it's above 4MB)
    uint32_t fb_addr = (uint32_t)fb.address;
    if (fb_addr >= 0x400000) {
        uint32_t pd_idx = fb_addr >> 22;
        page_directory[pd_idx] = (fb_addr & 0xFFC00000) | 0x83;
    }

    // 4. Load Page Directory and Enable Paging
    asm volatile("mov %0, %%cr3" :: "r"(page_directory));

    uint32_t cr4;
    asm volatile("mov %%cr4, %0" : "=r"(cr4));
    cr4 |= 0x00000010; // Enable PSE
    asm volatile("mov %0, %%cr4" :: "r"(cr4));

    uint32_t cr0;
    asm volatile("mov %%cr0, %0" : "=r"(cr0));
    cr0 |= 0x80000000; // Enable Paging
    asm volatile("mov %0, %%cr0" :: "r"(cr0));
}