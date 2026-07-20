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
void blend_color(uint32_t* bg, uint32_t fg, float alpha) {
    uint8_t src_r = (fg >> 16) & 0xFF;
    uint8_t src_g = (fg >> 8) & 0xFF;
    uint8_t src_b = fg & 0xFF;

    uint8_t dest_r = (*bg >> 16) & 0xFF;
    uint8_t dest_g = (*bg >> 8) & 0xFF;
    uint8_t dest_b = *bg & 0xFF;

    uint8_t blended_r = (uint8_t)(src_r * alpha + dest_r * (1 - alpha));
    uint8_t blended_g = (uint8_t)(src_g * alpha + dest_g * (1 - alpha));
    uint8_t blended_b = (uint8_t)(src_b * alpha + dest_b * (1 - alpha));

    *bg = (blended_r << 16) | (blended_g << 8) | blended_b;
}