#include <stdint.h>
#include <stdbool.h>
#include "../include/portio.h"

extern void terminal_putchar(char c);

static const char keymap[128] = {
    0, 27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', 8,
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', 0,
    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0, '\\',
    'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' '
};

static const char keymap_shift[128] = {
    0, 27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', 8,
    '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n', 0,
    'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~', 0, '|',
    'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0, '*', 0, ' '
};

static bool shift = false;
static bool caps = false;

void keyboard_input(uint8_t scancode) {
    if (scancode & 0x80) { // Key release
        if ((scancode & 0x7F) == 0x2A || (scancode & 0x7F) == 0x36)
            shift = false;
        return;
    }

    if (scancode == 0x2A || scancode == 0x36) { // Shift press
        shift = true;
        return;
    }

    if (scancode == 0x3A) { // Caps Lock
        caps = !caps;
        return;
    }

    char c = (shift ^ caps) ? keymap_shift[scancode] : keymap[scancode];
    if (c) terminal_putchar(c);
}

void irq1_handler(void) {
    uint8_t scancode = inb(0x60);
    keyboard_input(scancode);
    // EOI is handled by the Assembly stub
}