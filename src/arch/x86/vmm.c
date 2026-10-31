#include "../../include/vmm.h"
#include "../../include/pmm.h"
#include "../../include/kstring.h"
#include <stdint.h>

// Provided by paging.c
extern uint32_t page_directory[1024];

#define PAGE_SIZE 4096
#define PAGE_MASK 0xFFFFF000

static inline void vmm_flush_tlb(void* addr) {
    asm volatile("invlpg (%0)" :: "r"(addr) : "memory");
}

/**
 * Map one 4KB virtual page to a physical frame.
 * Both addresses MUST be page-aligned.
 */
// void vmm_map_page(void* virtual_addr, void* physical_addr, uint32_t flags) {
//     uint32_t va = (uint32_t)virtual_addr;
//     uint32_t pa = (uint32_t)physical_addr;

//     // 1️⃣ Enforce alignment (VMM contract)
//     if ((va & 0xFFF) || (pa & 0xFFF)) {
//         return; // invalid usage → silently refuse for now
//     }

//     uint32_t pd_idx = va >> 22;
//     uint32_t pt_idx = (va >> 12) & 0x3FF;

//     // 2️⃣ Ensure page table exists
//     if (!(page_directory[pd_idx] & VMM_PRESENT)) {

//         // Allocate a frame for page table
//         uint32_t pt_phys = (uint32_t)pmm_alloc_block();
//         if (!pt_phys) return; // out of memory

//         // Because low memory is identity-mapped, we can safely clear it
//         memset((void*)pt_phys, 0, PAGE_SIZE);

//         // Install page table (kernel RW)
//         page_directory[pd_idx] =
//             (pt_phys & PAGE_MASK) | VMM_PRESENT | VMM_WRITABLE;
//     }

//     // 3️⃣ Access page table via identity mapping
//     uint32_t* page_table =
//         (uint32_t*)(page_directory[pd_idx] & PAGE_MASK);

//     // 4️⃣ Install page mapping
//     if (page_table[pt_idx] & VMM_PRESENT) {
//         return;
//     }
//     page_table[pt_idx] =
//         (pa & PAGE_MASK) | flags | VMM_PRESENT;

//     // 5️⃣ Flush TLB entry
//     vmm_flush_tlb((void*)va);
// }
void vmm_map_page(void* virtual_addr, void* physical_addr, uint32_t flags) {
    uint32_t va = (uint32_t)virtual_addr;
    uint32_t pa = (uint32_t)physical_addr;

    if ((va & 0xFFF) || (pa & 0xFFF))
        return;

    uint32_t pd_idx = va >> 22;
    uint32_t pt_idx = (va >> 12) & 0x3FF;

    // PDE 1023 is reserved for recursive mapping
    if (pd_idx == 1023)
        return;

    // Create page table if it doesn't exist
    if (!(page_directory[pd_idx] & VMM_PRESENT)) {
        uint32_t pt_phys = (uint32_t)pmm_alloc_block();

        if (!pt_phys)
            return;

        page_directory[pd_idx] =
            (pt_phys & PAGE_MASK) | VMM_PRESENT | VMM_WRITABLE;

        uint32_t pt_virt =
            RECURSIVE_PT_BASE + (pd_idx * PAGE_SIZE);

        // Invalidate recursive page-table address
        vmm_flush_tlb((void*)pt_virt);

        // Clear the new page table
        memset((void*)pt_virt, 0, PAGE_SIZE);
    }

    // Access page table through recursive mapping
    uint32_t* page_table =
        (uint32_t*)(RECURSIVE_PT_BASE + pd_idx * PAGE_SIZE);

    if (page_table[pt_idx] & VMM_PRESENT)
        return;

    page_table[pt_idx] =
        (pa & PAGE_MASK) | flags | VMM_PRESENT;

    vmm_flush_tlb((void*)va);
}
/**
 * Unmap a virtual page.
 * NOTE: Does NOT free physical memory.
 */
// void vmm_unmap_page(void* virtual_addr) {
//     uint32_t va = (uint32_t)virtual_addr;

//     if (va & 0xFFF) return;

//     uint32_t pd_idx = va >> 22;
//     uint32_t pt_idx = (va >> 12) & 0x3FF;

//     if (!(page_directory[pd_idx] & VMM_PRESENT))
//         return;

//     uint32_t* page_table =
//         (uint32_t*)(page_directory[pd_idx] & PAGE_MASK);

//     // Remove mapping only
//     page_table[pt_idx] = 0;

//     vmm_flush_tlb((void*)va);
// }
void vmm_unmap_page(void* virtual_addr) {
    uint32_t va = (uint32_t)virtual_addr;

    if (va & 0xFFF)
        return;

    uint32_t pd_idx = va >> 22;
    uint32_t pt_idx = (va >> 12) & 0x3FF;

    if (pd_idx == 1023)
        return;

    if (!(page_directory[pd_idx] & VMM_PRESENT))
        return;

    uint32_t* page_table =
        (uint32_t*)(RECURSIVE_PT_BASE + pd_idx * PAGE_SIZE);

    page_table[pt_idx] = 0;

    vmm_flush_tlb((void*)va);
}

/**
 * Translate virtual → physical (debug/helper)
 */
// uint32_t vmm_get_phys(void* virtual_addr) {
//     uint32_t va = (uint32_t)virtual_addr;

//     uint32_t pd_idx = va >> 22;
//     uint32_t pt_idx = (va >> 12) & 0x3FF;
//     uint32_t offset = va & 0xFFF;

//     if (!(page_directory[pd_idx] & VMM_PRESENT))
//         return 0;

//     uint32_t* page_table =
//         (uint32_t*)(page_directory[pd_idx] & PAGE_MASK);

//     if (!(page_table[pt_idx] & VMM_PRESENT))
//         return 0;

//     return (page_table[pt_idx] & PAGE_MASK) + offset;
// }

uint32_t vmm_get_phys(void* virtual_addr) {
    uint32_t va = (uint32_t)virtual_addr;

    uint32_t pd_idx = va >> 22;
    uint32_t pt_idx = (va >> 12) & 0x3FF;
    uint32_t offset = va & 0xFFF;

    if (pd_idx == 1023)
        return 0;

    if (!(page_directory[pd_idx] & VMM_PRESENT))
        return 0;

    uint32_t* page_table =
        (uint32_t*)(RECURSIVE_PT_BASE + pd_idx * PAGE_SIZE);

    if (!(page_table[pt_idx] & VMM_PRESENT))
        return 0;

    return (page_table[pt_idx] & PAGE_MASK) + offset;
}