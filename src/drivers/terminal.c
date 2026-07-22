#include "../include/terminal.h"

struct terminal_property terminal_p;

void terminal_init(struct terminal_property *t)
{
    terminal_p = *t;

    rectangle_t rect;
    rect.width  = terminal_p.width;
    rect.height = terminal_p.height;

    draw_rectangle(
        &rect,
        terminal_p.x,
        terminal_p.y,
        0xC4C4C4,
        0xFFFFFF,
        0.5f
    );
    rect.height *= 0.05;
    title_bar(&rect,terminal_p.x,terminal_p.y,0x000000,0x000000,"Terminal >_");
}