#include "../include/pci.h"

extern void terminal_write(const char* s);
extern void terminal_write_hex(uint32_t n);

// Forward declaration of the internal init step
void pcnet_init_device(uint8_t bus, uint8_t slot);

void pcnet_scan(void)
{
    for (int bus = 0; bus < 256; bus++)
    {
        for (int slot = 0; slot < 32; slot++)
        {
            uint16_t vendor_id = pciConfigReadWord(bus, slot, 0, 0);

            if (vendor_id != 0xFFFF)
            {
                uint16_t device_id = pciConfigReadWord(bus, slot, 0, 2);

                terminal_write("Found device: Vendor ID = ");
                terminal_write_hex(vendor_id);
                terminal_write(", Device ID = ");
                terminal_write_hex(device_id);
                terminal_write("\n");

                if(vendor_id == 0x1022 && device_id == 0x2000) {
                    terminal_write("AMD PCnet device found at bus ");
                    terminal_write_hex(bus);
                    terminal_write(", slot ");
                    terminal_write_hex(slot);
                    terminal_write("\n");
                    
                    // Initialize the specific hardware instance found
                    pcnet_init_device(bus, slot);
                }
            }
        }
    }
}

void pcnet_init_device(uint8_t bus, uint8_t slot)
{
    // Read BAR0 (Offset 0x10) using the correct coordinates
    uint32_t io_base = pciConfigReadDword(bus, slot, 0, 0x10); 
    
    // Mask out the trailing PCI configuration bit flags 
    // Bit 0 specifies I/O space (1) or Memory space (0)
    uint32_t actual_io_port = io_base & ~0x3;

    terminal_write("AMD PCnet I/O Base Address: ");
    terminal_write_hex(actual_io_port);
    terminal_write("\n");
    uint32_t conf = pciConfigReadDword(bus, slot, 0, 0x04);

    conf |= 0x00000005;  // Enable I/O Space + Bus Mastering

    pciConfigWriteDword(bus, slot, 0, 0x04, conf);

    // Next step: Enable Bus Mastering and I/O Space in the PCI Command Register
    // uint16_t command = pciConfigReadWord(bus, slot, 0, 0x04);
    // pciConfigWriteWord(bus, slot, 0, 0x04, command | 0x05); 
    uint16_t reset = inw(actual_io_port + 0x14);
    terminal_write("AMD PCnet Reset Register Value: ");
    terminal_write_hex(reset);
    terminal_write("\n");
    outw(actual_io_port + 0x12, 0000);
    uint16_t csr0 = inw(actual_io_port + 0x10);
    terminal_write("AMD PCnet CSR0 Register Value: ");
    terminal_write_hex(csr0);
    terminal_write("\n");
    if(csr0 & 0x0004) {
        terminal_write("stoped.\n");
    } else {
        terminal_write("running.\n");
    }
}

void pcnet_init(void)
{
    terminal_write("Initializing AMD PCnet driver...\n");
    pcnet_scan();
}
