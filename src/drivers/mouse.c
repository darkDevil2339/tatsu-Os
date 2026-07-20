#include "../include/mouse.h"
#include "../include/portio.h"
#include "../include/framebuffer.h"
#include "../include/window.h"


int mouse_x = 400; 
int mouse_y = 300;
bool mouse_left_button_pressed;
bool mouse_right_button_pressed;
bool mouse_middle_button_pressed;
bool mouse_left_button_prev;
bool mouse_right_button_prev;
bool mouse_middle_button_prev;

static uint8_t packet[3];
static uint8_t cycle = 0;
rectangle_t r = {100 * 4, 60 * 4};
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
    mouse_left_button_prev = mouse_left_button_pressed;
    mouse_right_button_prev = mouse_right_button_pressed;
    mouse_middle_button_prev = mouse_middle_button_pressed;
    mouse_left_button_pressed = (packet[0] & 0x01) != 0;
    mouse_right_button_pressed = (packet[0] & 0x02) != 0;
    mouse_middle_button_pressed = (packet[0] & 0x04) != 0;
    if(mouse_left_button_pressed && !mouse_left_button_prev) {
        draw_rectangle(&r, fb.width / 2 - r.width / 2, fb.height / 2 - r.height / 2, 0xFF0080, 0.5f);
        // terminal_write("Mouse Left Button Clicked at ");
        // terminal_write_hex(mouse_x);
        // terminal_write(", ");
        // terminal_write_hex(mouse_y);
        // terminal_write("\n");
    }

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
