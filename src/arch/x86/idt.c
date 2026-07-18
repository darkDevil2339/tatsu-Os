#include <stdint.h>
#include "../../include/portio.h"

/* IDT entry structure */
struct idt_entry {
    uint16_t base_low;
    uint16_t sel;
    uint8_t  always0;
    uint8_t  flags;
    uint16_t base_high;
} __attribute__((packed));

struct idt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

/* ASM functions */
extern void idt_flush(uint32_t);
extern void irq1_handler_stub();
extern void irq12_handler_stub();

struct idt_entry idt[256];
struct idt_ptr idtp;

static void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags) {
    idt[num].base_low  = base & 0xFFFF;
    idt[num].base_high = (base >> 16) & 0xFFFF;
    idt[num].sel       = sel;
    idt[num].always0   = 0;
    idt[num].flags     = flags;
}

void idt_init(void) {
    idtp.limit = sizeof(idt) - 1;
    idtp.base  = (uint32_t)&idt;

    /* Clear IDT */
    for (int i = 0; i < 256; i++)
        idt_set_gate(i, 0, 0, 0);

    /* ================= PIC REMAP FIRST (CRITICAL FIX) ================= */

    outb(0x20, 0x11); io_wait();
    outb(0xA0, 0x11); io_wait();

    outb(0x21, 0x20); io_wait(); // Master offset 0x20
    outb(0xA1, 0x28); io_wait(); // Slave offset 0x28

    outb(0x21, 0x04); io_wait();
    outb(0xA1, 0x02); io_wait();

    outb(0x21, 0x01); io_wait();
    outb(0xA1, 0x01); io_wait();

    /* ================= NOW install IRQ gates ================= */

    /* IRQ1 -> INT 33 (0x21) */
    idt_set_gate(33, (uint32_t)irq1_handler_stub,  0x08, 0x8E);

    /* IRQ12 -> INT 44 (0x2C) */
    idt_set_gate(44, (uint32_t)irq12_handler_stub, 0x08, 0x8E);

    /* Enable keyboard + mouse IRQs only */
   outb(0x21, 0xF9);  // 11111001 → enable IRQ1 + IRQ12
    outb(0xA1, 0xEF);  // enable IRQ12


    /* Load IDT */
    idt_flush((uint32_t)&idtp);
}
