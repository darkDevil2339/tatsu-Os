#include <stdint.h>
#define KERNEL_VIRT_BASE 0xC0000000

extern void kernel_higher_half_main(void);

__attribute__((noreturn))
void jump_to_higher_half(void) {
    
    uint32_t esp;
    asm volatile ("mov %%esp, %0" : "=r"(esp));

    // Shift stack pointer to the Virtual address space
    esp += KERNEL_VIRT_BASE;

    asm volatile (
        "mov %0, %%esp\n"   // Set new stack
        "push %1\n"         // Push virtual address of the next function
        "ret\n"             // 'Return' to that address (forces absolute jump)
        :
        : "r"(esp), "r"(&kernel_higher_half_main)
    );
    while(1);
}