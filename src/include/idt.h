#pragma once
#include <stdint.h>

// IDT entry
typedef struct {
    uint16_t base_low;
    uint16_t sel;
    uint8_t  always0;
    uint8_t  flags;
    uint16_t base_high;
} __attribute__((packed)) idt_entry_t;

// IDT pointer
typedef struct {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed)) idt_ptr_t;

// Declare IDT and pointer (global for higher-half mapping)
extern idt_entry_t idt[256];
extern idt_ptr_t idt_ptr;

// Assembly wrapper to load IDT
void lidt(idt_ptr_t* idt_ptr);
