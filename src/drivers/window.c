#include "../include/window.h"

struct rectangle_t rect;
void draw_rectangle(rectangle_t* rect, uint32_t x, uint32_t y, uint32_t color) {
    for (uint32_t i = 0; i < rect->height; i++) {
        for (uint32_t j = 0; j < rect->width; j++) {
            rect->pixels[(y + i) * rect->width + (x + j)] = color;
        }
    }
}

void draw_line(rectangle_t* rect, uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2, uint32_t color) {
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;
    int err = dx - dy;

    while (1) {
        if (x1 >= 0 && x1 < rect->width && y1 >= 0 && y1 < rect->height) {
            rect->pixels[y1 * rect->width + x1] = color;
        }

        if (x1 == x2 && y1 == y2) {
            break;
        }

        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x1 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y1 += sy;
        }
    }
}

void window_init() {
    // Initialization code for the window
    rect.width = 800;  // Example width
    rect.height = 600; // Example height
    rect.pixels = (uint32_t*)malloc(rect.width * rect.height * sizeof(uint32_t));
    if (!rect.pixels) {
        // Handle memory allocation failure
        return;
    }
    draw_rectangle(&rect, 0, 0, 0xFFFFFF); // Example: Draw a white rectangle
}