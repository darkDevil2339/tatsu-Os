#include "../include/pci.h"
extern void terminal_write(const char* s);
extern void terminal_write_hex(uint32_t n);
void pcnet_scan(void)
{
    for (int bus = 0; bus < 256; bus++)
    {
        for (int slot = 0; slot < 32; slot++)
        {
            uint16_t vendor_id = pciConfigReadWord(bus, slot, 0, 0);

            if (vendor_id != 0xFFFF)
            {
                uint16_t device_id =
                    pciConfigReadWord(bus, slot, 0, 2);

                terminal_write("Found device: Vendor ID = ");
                terminal_write_hex(vendor_id);
                terminal_write(", Device ID = ");
                terminal_write_hex(device_id);
                terminal_write("\n");
            }
        }
    }
}
