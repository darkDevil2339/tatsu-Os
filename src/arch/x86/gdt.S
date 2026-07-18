# gdt.S — GAS (AT&T)
# --------------------------------
# void gdt_flush(uint32_t gdt_ptr)
# --------------------------------

    .global gdt_flush

gdt_flush:
    movl 4(%esp), %eax        # get gdt_ptr
    lgdt (%eax)               # load GDT

    movw $0x10, %ax           # data selector
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs
    movw %ax, %ss

    ljmp $0x08, $flush_cs     # reload CS
flush_cs:
    ret
