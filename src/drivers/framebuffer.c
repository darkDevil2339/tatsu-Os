#include "../include/framebuffer.h"
#include "../include/multiboot.h"
#include <stddef.h>

framebuffer_t fb = {0};

void framebuffer_init(multiboot_info_t* mb_info) {
    // Check if the bootloader actually provided framebuffer info
    if (!(mb_info->flags & (1 << 12))) {
        fb.ready = 0;
        return;
    }

    fb.address = (void*)(uintptr_t)(mb_info->framebuffer_addr);
    fb.width   = mb_info->framebuffer_width;
    fb.height  = mb_info->framebuffer_height;
    fb.pitch   = mb_info->framebuffer_pitch;
    fb.bpp     = mb_info->framebuffer_bpp;
    
    if (fb.address != NULL) {
        fb.ready = 1; //
    }
}

void framebuffer_putpixel(unsigned int x, unsigned int y, uint32_t color) {
    if (!fb.ready || x >= fb.width || y >= fb.height) return;

    // Calculate pixel offset: (y * pitch) + (x * bytes_per_pixel)
    uint8_t* pixel = (uint8_t*)fb.address + (y * fb.pitch) + (x * (fb.bpp / 8));
    *(uint32_t*)pixel = color;
}

void framebuffer_clear(uint32_t color) {
    for (unsigned int y = 0; y < fb.height; y++) {
        for (unsigned int x = 0; x < fb.width; x++) {
            framebuffer_putpixel(x, y, color);
        }
    }
}

void read_pixel(unsigned int x, unsigned int y, uint32_t* color) {
    if (!fb.ready || x >= fb.width || y >= fb.height) return;

    uint8_t* pixel = (uint8_t*)fb.address + (y * fb.pitch) + (x * (fb.bpp / 8));
    *color = *(uint32_t*)pixel;
}  