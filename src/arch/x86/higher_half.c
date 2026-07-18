// #include <stdint.h>
// #include "../include/vmm.h"
// #include "../include/framebuffer.h"

// #define KERNEL_VIRT_BASE 0xC0000000
// #define PAGE_SIZE 4096

// // Symbols from linker.ld
// extern uint32_t __kernel_start; //
// extern uint32_t __kernel_end;   //
// extern uint32_t stack_start;    //
// extern uint32_t stack_end;      //


// /*
//  * Maps the physical kernel and stack to the Higher Half (0xC0000000+)
//  */
// void higher_half_map(void) {
//     // 1. Map the kernel body (text, rodata, data, bss)
//     // __kernel_start is at 2M
    
//     uint32_t phys_start = (uint32_t)&__kernel_start;
//     uint32_t phys_end   = (uint32_t)&__kernel_end;

//     for (uint32_t phys = phys_start; phys < phys_end; phys += PAGE_SIZE) {
//         void* virt_addr = (void*)(phys + KERNEL_VIRT_BASE);
//         vmm_map_page(virt_addr, (void*)phys, VMM_PRESENT | VMM_WRITABLE);
//     }

//     // 2. Map the 16KB stack
//     uint32_t s_phys_start = (uint32_t)&stack_start;
//     uint32_t s_phys_end   = (uint32_t)&stack_end;

//     for (uint32_t phys = s_phys_start; phys < s_phys_end; phys += PAGE_SIZE) {
//         void* virt_addr = (void*)(phys + KERNEL_VIRT_BASE);
//         vmm_map_page(virt_addr, (void*)phys, VMM_PRESENT | VMM_WRITABLE);
//     }
// //     if (fb.ready) {
// //     uint32_t fb_phys = (uint32_t)fb.address;
// //     uint32_t fb_size = fb.pitch * fb.height;

// //     for (uint32_t off = 0; off < fb_size; off += PAGE_SIZE) {
// //         vmm_map_page(
// //             (void*)(fb_phys + off + KERNEL_VIRT_BASE), // virtual HH addr
// //             (void*)(fb_phys + off),                     // physical addr
// //             VMM_PRESENT | VMM_WRITABLE
// //         );
// //     }

// //     // THIS IS CRITICAL — update the framebuffer pointer
// //     fb.address = (void*)(fb_phys + KERNEL_VIRT_BASE);
// // }


// }
#include <stdint.h>
#include "../../include/vmm.h"
#include "../../include/framebuffer.h"
#include "../../include/pmm.h"
#include "../../include/kstring.h"

#define KERNEL_VIRT_BASE 0xC0000000
#define PAGE_SIZE 4096

extern uint32_t __kernel_start;
extern uint32_t __kernel_end;
extern uint32_t stack_start;
extern uint32_t stack_end;
extern framebuffer_t fb;

/* Map kernel, stack, and framebuffer to higher half */
void higher_half_map(void) {
    uint32_t phys_start = (uint32_t)&__kernel_start;
    uint32_t phys_end   = (uint32_t)&__kernel_end;

    // 1️⃣ Map kernel
    for (uint32_t phys = phys_start; phys < phys_end; phys += PAGE_SIZE) {
        vmm_map_page((void*)(phys + KERNEL_VIRT_BASE),
                     (void*)phys,
                     VMM_PRESENT | VMM_WRITABLE);
    }

    // 2️⃣ Map stack
    uint32_t s_phys = (uint32_t)&stack_start;
    uint32_t s_end  = (uint32_t)&stack_end;
    for (uint32_t phys = s_phys; phys < s_end; phys += PAGE_SIZE) {
        vmm_map_page((void*)(phys + KERNEL_VIRT_BASE),
                     (void*)phys,
                     VMM_PRESENT | VMM_WRITABLE);
    }

    // 3️⃣ Map framebuffer
    for (uint32_t off = 0; off < fb.pitch * fb.height; off += PAGE_SIZE) {
        vmm_map_page(
            (void*)((uint32_t)fb.address + KERNEL_VIRT_BASE + off),
            (void*)((uint32_t)fb.address + off),
            VMM_PRESENT | VMM_WRITABLE
        );
    }

    // 4️⃣ Update framebuffer pointer to higher half
    fb.address = (void*)((uint32_t)fb.address + KERNEL_VIRT_BASE);
}
