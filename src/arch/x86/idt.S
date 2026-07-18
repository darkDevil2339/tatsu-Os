# idt.S — GAS (AT&T syntax)
# --------------------------------
# void idt_flush(uint32_t idt_ptr)
# idt_ptr -> struct { uint16_t limit; uint32_t base; }
# --------------------------------

    .global idt_flush

idt_flush:
    movl 4(%esp), %eax        # get idt_ptr argument
    lidt (%eax)               # load IDT register
    ret
