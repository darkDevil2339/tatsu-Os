#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include <stdint.h>
#include "multiboot.h"

typedef struct {
    void* address;
    uint32_t width;
    uint32_t height;
    uint32_t pitch;
    uint32_t bpp;
    int      ready;
} framebuffer_t;

extern framebuffer_t fb;

void framebuffer_init(multiboot_info_t* mb_info);
void framebuffer_putpixel(unsigned int x, unsigned int y, uint32_t color);
void framebuffer_clear(uint32_t color);
void read_pixel(unsigned int x, unsigned int y, uint32_t* color);

#endif