ENTRY(_start)

SECTIONS
{
    . = 2M;

    __kernel_start = .;  /* start of kernel */

    .text BLOCK(4K) : ALIGN(4K)
    {
        *(.multiboot)
        *(.text)
    }

    .rodata BLOCK(4K) : ALIGN(4K)
    {
        *(.rodata)
    }

    .data BLOCK(4K) : ALIGN(4K)
    {
        *(.data)
    }

    .bss BLOCK(4K) : ALIGN(4K)
    {
        *(COMMON)
        *(.bss)
    }

    __kernel_end = .;    /* end of kernel */

    .stack BLOCK(4K) : ALIGN(4K)
    {
        stack_start = .;   /* symbol for stack start */
        . += 16K;          /* 16 KB stack size */
        stack_end = .;     /* symbol for stack end */
    }
}
