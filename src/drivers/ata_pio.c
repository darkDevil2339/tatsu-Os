#include<stdint.h>
#include "../include/portio.h"

extern void terminal_write(const char* s);
extern void terminal_write_hex(uint32_t n);
void ata_wait_busy() {
    int timeout = 1000000; // Arbitrary large timeout to prevent infinite loop
    while((inb(0x1F7) & 0x80) && timeout-- > 0);
}
void ata_wait_drq()
{
    uint8_t status;
    int timeout = 1000000;

    while(timeout--)
    {
        status = inb(0x1F7);

        if(status & 0x01) // ERR bit
        {
            terminal_write("ATA ERROR\n");
            return;
        }

        if(status & 0x08) // DRQ
            return;
    }

    terminal_write("ATA TIMEOUT\n");
}

void ata_read_sector(uint32_t lba, uint8_t* buffer)
{
    ata_wait_busy();

    outb(0x1F6, 0xE0 | ((lba >> 24) & 0x0F));
    outb(0x1F2, 1);
    outb(0x1F3, lba & 0xFF);
    outb(0x1F4, (lba >> 8) & 0xFF);
    outb(0x1F5, (lba >> 16) & 0xFF);
    outb(0x1F7, 0x20);

    inb(0x1F7); inb(0x1F7); inb(0x1F7); inb(0x1F7);

    ata_wait_drq();

    for(int i = 0; i < 256; i++)
    {
        ((uint16_t*)buffer)[i] = inw(0x1F0);
    }

    for(int i = 0; i < 16; i++)
    {
        terminal_write_hex(buffer[i]);
        terminal_write(" ");
    }
    if(lba == 0){
         terminal_write(" (MBR)\n");
         if(buffer[510] == 0x55 && buffer[511] == 0xAA)
            terminal_write("Valid MBR signature found!\n");
        else{
        terminal_write("Invalid MBR signature!\n");
        }
    }
    else{
        terminal_write("\n");
    }
    

    terminal_write("\n");
}

void ata_write_sector(uint32_t lba, uint8_t* buffer)
{
    ata_wait_busy();

    outb(0x1F6, 0xE0 | ((lba >> 24) & 0x0F));
    outb(0x1F2, 1);
    outb(0x1F3, lba & 0xFF);
    outb(0x1F4, (lba >> 8) & 0xFF);
    outb(0x1F5, (lba >> 16) & 0xFF);
    outb(0x1F7, 0x30);

    inb(0x1F7); inb(0x1F7); inb(0x1F7); inb(0x1F7);

    ata_wait_drq();

    for(int i = 0; i < 256; i++)
    {
        outw(0x1F0, ((uint16_t*)buffer)[i]);
    }

    terminal_write("Wrote sector to disk using ATA PIO!\n");
}
