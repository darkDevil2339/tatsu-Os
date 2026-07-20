#ifndef MOUSE_H
#define MOUSE_H

#include <stdint.h>
#include <stdbool.h>

extern int mouse_x;
extern int mouse_y;
extern bool mouse_left_button_pressed;
extern bool mouse_right_button_pressed;
extern bool mouse_middle_button_pressed;
extern bool mouse_left_button_prev;
extern bool mouse_right_button_prev;
extern bool mouse_middle_button_prev;

void mouse_init(void);
void mouse_irq_handler(uint8_t data);

#endif
