#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "../include/framebuffer.h"
#include "../include/multiboot.h"
#include "../include/portio.h"
#include "../include/mouse.h"
#include "../include/bitmap.h"
#include "../include/logo.h"
#include "../include/paging.h"
#include "../include/pmm.h"
#include "../include/higher_half.h"
#include "../include/vmm.h"
#include "../include/ata_pio.h"
#include "../include/kstring.h"
#include "../include/background.h"

#define Author "Sachin Kumar"

/* ===== Forward declarations ===== */
void gdt_init(void);
void idt_init(void);
void terminal_write(const char* s);
void terminal_putchar(char c);
void execute_command(const char *input, int arg_int, char *arg_str);
void mouse_init(void);
void fade_in_logo(void);
void delay_ms(uint32_t ms);

extern uint32_t multiboot_ptr;
extern uint32_t multiboot_magic;
extern uint32_t __kernel_start;
extern uint32_t __kernel_end;
extern uint32_t page_directory[1024];

/* Full 8x8 font bitmap */
extern char font8x8_basic[128][8]; 

uint32_t cursor_x = 0;
uint32_t cursor_y = 0;
uint32_t text_color = 0xFFFFFF; // White

/* ===== Physical Memory Manager Initialization from Multiboot ===== */
void pmm_init_from_multiboot(multiboot_info_t* mbi) {
    multiboot_mmap_entry_t* mmap =
        (multiboot_mmap_entry_t*)mbi->mmap_addr;

    while ((uint32_t)mmap < mbi->mmap_addr + mbi->mmap_length) {

        if (mmap->type == 1) { // USABLE RAM
            uint64_t start = mmap->addr;
            uint64_t end   = mmap->addr + mmap->len;

            // Align to PMM block size
            if (start % PMM_BLOCK_SIZE)
                start += PMM_BLOCK_SIZE - (start % PMM_BLOCK_SIZE);

            for (uint64_t addr = start;
                 addr < end;
                 addr += PMM_BLOCK_SIZE) {

                pmm_mark_free((uint32_t)addr);
            }
        }

        mmap = (multiboot_mmap_entry_t*)
            ((uint32_t)mmap + mmap->size + sizeof(uint32_t));
    }

    // 1 Mark kernel as USED
    for (uint32_t addr = (uint32_t)&__kernel_start;
         addr < (uint32_t)&__kernel_end;
         addr += PMM_BLOCK_SIZE) {
        pmm_mark_used(addr);
    }

    // 2️ Mark PMM bitmap itself as USED
    uint32_t bitmap_start = (uint32_t)pmm_bitmap;
    uint32_t bitmap_end   = bitmap_start + pmm_bitmap_size;

    for (uint32_t addr = bitmap_start;
         addr < bitmap_end;
         addr += PMM_BLOCK_SIZE) {
        pmm_mark_used(addr);
    }
}


/* 1. Added terminal_write  */
void terminal_write_hex(uint32_t n) {
    char hex_chars[] = "0123456789ABCDEF";
    char buffer[9];
    buffer[8] = '\0';
    for (int i = 7; i >= 0; i--) {
        buffer[i] = hex_chars[n & 0xF];
        n >>= 4;
    }
    terminal_write(buffer);
}
void terminal_write(const char* s) {
    for (size_t i = 0; s[i] != '\0'; i++) {
        terminal_putchar(s[i]);
    }
}

/* 2.  terminal_putchar logic */
void terminal_putchar(char c) {
    if (c == '\n') {
        cursor_x = 0;
        cursor_y += 12;
        return;
    }

    if (c == '\b') {
        if (cursor_x >= 8) cursor_x -= 8;
        for (int py = 0; py < 8; py++)
            for (int px = 0; px < 8; px++)
                framebuffer_putpixel(cursor_x + px, cursor_y + py, 0x000000); // Match background
        return;
    }

    // Draw character: bits are usually stored most-significant-bit first (left-to-right)
    for (int py = 0; py < 8; py++) {
        for (int px = 0; px < 8; px++) {
            if (font8x8_basic[(int)c][py] & (1 << px)) {
                framebuffer_putpixel(cursor_x + px, cursor_y + py, text_color);
            }
        }
    }

    cursor_x += 8;
    if (cursor_x + 8 > fb.width) {
        cursor_x = 0;
        cursor_y += 12;
    }
}

/* 3. Mouse Stubs */
/* 3. Mouse Handlers */
uint32_t saved_pixel_colors[25]; // To save background pixels
void draw_mouse_cursor(uint32_t x, uint32_t y) {
    // Draws a 5x5 red square
    for(int i = 0; i < 5; i++) {
        for(int j = 0; j < 5; j++) {
            // Ensure we don't draw off-screen to avoid crashes
            if ((x + i) < fb.width && (y + j) < fb.height) {
                read_pixel(x+i, y+j, &saved_pixel_colors[i * 5 + j]);
                framebuffer_putpixel(x + i, y + j, 0xFF0000); 
            }
        }
    }
}

void erase_mouse_cursor(uint32_t x, uint32_t y) {
    // IMPORTANT: This must match  background color exactly.
    for(int i = 0; i < 5; i++) {
        for(int j = 0; j < 5; j++) {
            if ((x + i) < fb.width && (y + j) < fb.height) {
                framebuffer_putpixel(x + i, y + j, saved_pixel_colors[i * 5 + j]);
            }
        }
    }
}

/* 4. Boot Logo */
void delay_seconds(uint32_t seconds) {
    for (uint32_t s = 0; s < seconds; s++) {
        for (volatile uint32_t i = 0; i < 50000000; i++);
    }
}

void display_logo() {
    fade_in_logo();
    delay_seconds(2);

}


void fade_in_logo() {
    for (uint32_t step = 0; step <= 255; step += 5) {
        framebuffer_clear(0x000000);

        for (uint32_t y = 0; y < logo_height; y++) {
            for (uint32_t x = 0; x < logo_width; x++) {
                uint32_t color = logoPixels[y * logo_width + x];
                if (color == 0x000000) continue;

                uint8_t r = (color >> 16) & 0xFF;
                uint8_t g = (color >> 8) & 0xFF;
                uint8_t b = color & 0xFF;

                r = (r * step) / 255;
                g = (g * step) / 255;
                b = (b * step) / 255;

                framebuffer_putpixel(
                    (fb.width - logo_width) / 2 + x,
                    (fb.height - logo_height) / 2 + y,
                    (r << 16) | (g << 8) | b
                );
            }
        }

        delay_seconds(1);  // smooth fade
    }
}
void background_init() {
    for(uint32_t step = 0; step <= 255; step += 5) {
        uint32_t color = (step << 16) | (step << 8) | (step / 2); // Gradually change 
        framebuffer_clear(color);
        delay_seconds(1);
    }
}

static inline void flush_tlb(void) {
    asm volatile (
        "mov %%cr3, %%eax\n"
        "mov %%eax, %%cr3\n"
        :
        :
        : "eax"
    );
}

#define PAGE_DIR_VADDR 0xFFFFF000

void unmap_identity_safe(void) {
    asm volatile("cli");

    uint32_t* pd = (uint32_t*)PAGE_DIR_VADDR;

    // Keep first PD entry (0x0 - 4MB) for I/O, terminal globals
    for (int i = 1; i < 1024; i++)
        pd[i] = 0;

    // Flush TLB
    asm volatile("mov %%cr3, %%eax; mov %%eax, %%cr3" ::: "eax");

    asm volatile("sti");
}



void kernel_main(void) {
    gdt_init();
    framebuffer_init((multiboot_info_t*)multiboot_ptr);
    fb.ready = 1;
    terminal_write("fb.ready = ");
    terminal_write_hex(fb.ready);
    terminal_write("\n");

    paging_init(); // Identity + recursive
    multiboot_info_t* mbi = (multiboot_info_t*)multiboot_ptr;
    uint32_t bitmap_addr = (uint32_t)&__kernel_end + 0x1000;
    uint32_t mem_size = (mbi->mem_upper * 1024) + 0x100000;

    pmm_init(mem_size, bitmap_addr);
    pmm_init_from_multiboot(mbi);

    higher_half_map(); // map kernel, stack, framebuffer
    jump_to_higher_half();
}

void kernel_higher_half_main(void) {
    __attribute__((packed))
    struct fs
    {
        uint16_t id;
        char name[16];
    };
    struct fs myfs = {1, "sachinfs"};
    uint8_t buffers[256];
    memcpy(buffers, &myfs, sizeof(myfs));
    asm volatile("cli");
    gdt_init();
    idt_init();

    terminal_write("Entering Higher Half at 0xC0000000...\n");

    // fb.address is already updated in higher_half_map()
    if (fb.ready) {
        display_logo();
        framebuffer_clear(0xFFFF22); // Match logo background
        background_init();
    }

    mouse_init();
    // keyboard_init();  // must call this
    // unmap_identity_safe();

    terminal_write("Framebuffer at: ");
    terminal_write_hex((uint32_t)fb.address);
    terminal_write("\n");
    terminal_write("Sachin OS: Higher Half Boot Successful\n> ");
    ata_read_sector(0, (uint8_t*)0x8000); // Test ATA PIO read
    // ata_write_sector(50, buffers); // Test ATA PIO write
    ata_read_sector(50, (uint8_t*)0x8000); // Verify write
    struct fs read_back;
    memcpy(&read_back, (void*)0x8000, sizeof(read_back));
    terminal_write("Read back from disk: id=");
    terminal_write_hex(read_back.id);
    terminal_write(", name=");
    terminal_write(read_back.name);
    terminal_write("\n");

    asm volatile("sti");

    while (1) asm volatile("hlt");
}


