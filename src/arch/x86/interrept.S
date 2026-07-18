# interrupts.S
.section .text
.align 4

.extern irq1_handler       # In keyboard.c
.extern mouse_irq_handler  # In mouse.c

.global irq1_handler_stub
.global irq12_handler_stub

# Keyboard Stub (IRQ 1)
irq1_handler_stub:
    pusha                  # Save all registers
    call irq1_handler      # Call C handler
    
    # Send EOI to Master PIC
    movb $0x20, %al
    outb %al, $0x20
    
    popa                   # Restore registers
    iret                   # Return from interrupt

# Mouse Stub (IRQ 12)
irq12_handler_stub:
    pusha
    
    # Read mouse data from port 0x60 directly in Assembly
    xorl %eax, %eax
    inb $0x60, %al
    
    # Pass data as the first argument: mouse_irq_handler(data)
    pushl %eax
    call mouse_irq_handler
    addl $4, %esp          # Clean up stack
    
    # Send EOI to both Slave and Master PIC (IRQ 12 is on Slave)
    movb $0x20, %al
    outb %al, $0xA0        # Slave PIC
    outb %al, $0x20        # Master PIC
    
    popa

    iret

# Blank line here to avoid "end of file" warning
