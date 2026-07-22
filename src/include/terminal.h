#ifndef TERMINAL_H
#define TERMINAL_H
#include "../include/window.h"

struct terminal_property{
    int x;
    int y;
    uint32_t height;
    uint32_t width;
    char title[];
};

void terminal_init(struct terminal_property *t);

#endif // TERMINAL_H