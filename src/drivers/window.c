#include "../include/window.h"

rectangle_t rect;
void draw_rectangle(rectangle_t* rect, uint32_t x, uint32_t y, uint32_t color, float alpha) {
    for (uint32_t i = 0; i < rect->height; i++) {
        for (uint32_t j = 0; j < rect->width; j++) {
            uint32_t pixel_color = color;
            uint32_t saved_pixel[100];
            read_pixel(x + j, y + i, &saved_pixel[0]);
            blend_color(&pixel_color, saved_pixel[0], alpha);
            framebuffer_putpixel(x + j, y + i, pixel_color);
        }
    }
}
void update_window(rectangle_t* rect, uint32_t x, uint32_t y, uint32_t color, float alpha) {
    for (uint32_t i = 0; i < rect->height; i++) {
        for (uint32_t j = 0; j < rect->width; j++) {
            uint32_t pixel_color = color;
            uint32_t saved_pixel_colors[100];
            read_pixel(x + j, y + i, &saved_pixel_colors[i * rect->width + j]);
            blend_color(&pixel_color, saved_pixel_colors[i * rect->width + j], alpha);
            framebuffer_putpixel(x + j, y + i, pixel_color);
        }
    }
}
void text_box( uint32_t x, uint32_t y, const char* text, uint32_t text_color) {
    for (size_t i = 0; text[i] != '\0'; i++) {
        char c = text[i];
        for (int py = 0; py < 8; py++) {
            for (int px = 0; px < 8; px++) {
                if (font8x8_basic[(int)c][py] & (1 << px)) {
                    framebuffer_putpixel(x + px+i*8, y + py, text_color);
                }
            }
        }
    }
}
void window_init() {
    // Initialization code for the window
    rect.width = 100*4;  // Example width
    rect.height = 60*4; // Example height 564957114288
    draw_rectangle(&rect, fb.width / 2 - rect.width / 2, fb.height / 2 - rect.height / 2, 0x800080, 0.5f);
    rect.width = 160;
    rect.height = 40; 
    draw_rectangle(&rect, fb.width / 2 - rect.width / 2, fb.height / 2 - rect.height / 2, 0xFFFFFF, 0.6f);
    text_box(fb.width / 2 - rect.width / 2 + 10, fb.height / 2 - rect.height / 2 + 10, "Welcome !", 0x000000);
    text_box(fb.width / 2 - rect.width / 2 + 10, fb.height / 2 - rect.height / 2 + 30, "Sachin OS", 0x000000);

}