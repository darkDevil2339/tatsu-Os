#include "../include/mouse.h"
#include "../include/portio.h"
#include "../include/framebuffer.h"

int mouse_x = 400; 
int mouse_y = 300;

static uint8_t packet[3];
static uint8_t cycle = 0;

extern void draw_mouse_cursor(int x, int y);
extern void erase_mouse_cursor(int x, int y);

void mouse_irq_handler(uint8_t data) {
    packet[cycle++] = data;

    if (cycle == 1 && !(packet[0] & 0x08)) {
        cycle = 0;
        return;
    }

    if (cycle < 3) return;
    cycle = 0;

    erase_mouse_cursor(mouse_x, mouse_y); // Fixed: added arguments

    mouse_x += (int8_t)packet[1];
    mouse_y -= (int8_t)packet[2]; 

    // Clamp to screen pixels instead of 79x24
    if (mouse_x < 0) mouse_x = 0;
    if (mouse_y < 0) mouse_y = 0;
    if (mouse_x > (int)fb.width - 5) mouse_x = fb.width - 5;
    if (mouse_y > (int)fb.height - 5) mouse_y = fb.height - 5;

    draw_mouse_cursor(mouse_x, mouse_y); // Fixed: added arguments
}

// ... keep your mouse_init, mouse_wait, and mouse_write functions ...




// Ensure you keep your mouse_init(), mouse_wait(), and mouse_write() functions here.
static void mouse_wait(uint8_t type) {
    uint32_t timeout = 100000;
    if (type == 0) {
        while (timeout-- && (inb(0x64) & 1) == 0);
    } else {
        while (timeout-- && (inb(0x64) & 2));
    }
}

static void mouse_write(uint8_t data) {
    mouse_wait(1);
    outb(0x64, 0xD4);
    mouse_wait(1);
    outb(0x60, data);
}

static uint8_t mouse_read(void) {
    mouse_wait(0);
    return inb(0x60);
}

void mouse_init(void) {
    mouse_wait(1);
    outb(0x64, 0xA8);        // enable mouse

    mouse_wait(1);
    outb(0x64, 0x20);
    mouse_wait(0);
    uint8_t status = inb(0x60) | 2;
    mouse_wait(1);
    outb(0x64, 0x60);
    mouse_wait(1);
    outb(0x60, status);

    mouse_write(0xF6); mouse_read();
    mouse_write(0xF4); mouse_read();
}
