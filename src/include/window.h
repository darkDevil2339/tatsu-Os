#ifndef Window_H
#define Window_H
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "../include/framebuffer.h"
#include "../include/heap.h"
#include "../include/mouse.h"
#include "../include/bitmap.h"


typedef struct {
    uint32_t width;
    uint32_t height;
} rectangle_t;
typedef struct {
    uint32_t radius;
    uint32_t center_x;
    uint32_t center_y;
} circle_t;
void draw_rectangle(rectangle_t* rect, uint32_t x, uint32_t y, uint32_t color, uint32_t border_color,float alpha);
void update_window(rectangle_t* rect, uint32_t x, uint32_t y, uint32_t color, float alpha);
void text_box(float scale, uint32_t x, uint32_t y, const char* text, uint32_t text_color);
void title_bar(rectangle_t* rect, uint32_t x, uint32_t y, uint32_t color, uint32_t borber_color,char title[]);
void window_init();

#endif // Window_H
