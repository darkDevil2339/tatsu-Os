#ifndef Window_H
#define Window_H
#include <stdint.h>
typedef struct {
    uint32_t width;
    uint32_t height;
    uint32_t* pixels;
} rectangle_t;
typedef struct {
    uint32_t radius;
    uint32_t center_x;
    uint32_t center_y;
    uint32_t* pixels;
} circle_t;
void draw_rectangle(rectangle_t* rect, uint32_t x, uint32_t y, uint32_t color);
void draw_circle(circle_t* circ, uint32_t x, uint32_t y, uint32_t color);
void draw_line(rectangle_t* rect, uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2, uint32_t color);
void window_init();


#endif // Window_H
