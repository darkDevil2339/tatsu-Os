
.set ALIGN,    1<<0             # align loaded modules on page boundaries
.set MEMINFO,  1<<1             # provide memory map
.set VIDEO,    1<<2             # request a graphical video mode
.set FLAGS,    ALIGN | MEMINFO | VIDEO 
.set MAGIC,    0x1BADB002       # 'magic number' lets bootloader find the header
.set CHECKSUM, -(MAGIC + FLAGS) # checksum of above, to prove we are multiboot

.section .multiboot
.align 4
.long MAGIC
.long FLAGS
.long CHECKSUM

# Header fields required for the VIDEO flag
.long 0, 0, 0, 0, 0             # Address fields (unused for ELF files)
.long 0                         # 0 = Linear Framebuffer mode
.long 1024                      # Preferred Width
.long 768                       # Preferred Height
.long 32                        # Preferred Depth (BPP)


.section .data
.global multiboot_magic
.global multiboot_ptr
multiboot_magic: .long 0
multiboot_ptr:   .long 0

.section .bss
.align 16
.global stack_bottom
.global stack_top
stack_bottom:
.skip 16384                     # 16 KB stack
stack_top:

.section .text
.global _start
.type _start, @function
.extern kernel_main

_start:
    cli                         # Disable interrupts
    mov $stack_top, %esp        # Set up the stack pointer
    
    # Save Multiboot data into the variables defined above
    movl %eax, (multiboot_magic)
    movl %ebx, (multiboot_ptr)
    
    call kernel_main            # Enter the C kernel

.hang:
    hlt

    jmp .hang



.size _start, . - _start
