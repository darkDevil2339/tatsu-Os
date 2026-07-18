#ifndef PAGING_H
#define PAGING_H

#include <stdint.h>

extern uint32_t __kernel_start;
extern uint32_t __kernel_end;
extern uint32_t stack_start;
extern uint32_t stack_end;

void paging_init(void);


#endif
