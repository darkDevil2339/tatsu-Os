#ifndef VMM_H
#define VMM_H

#include <stdint.h>
#include <stddef.h>

// Standard x86 Paging Flags
#define VMM_PRESENT 0x01
#define VMM_WRITABLE 0x02
#define VMM_USER 0x04
#define PAGE_SIZE 4096
#define KERNEL_VIRT_BASE 0xC0000000

void vmm_map_page(void* virtual_addr, void* physical_addr, uint32_t flags);
void vmm_unmap_page(void* virtual_addr);

#endif