#include "../include/pci.h"
#include "../include/amd_pcnet.h"
#include "../include/pmm.h"

extern void terminal_write(const char* s);
extern void terminal_write_hex(uint32_t n);

void pcnet_init_device(uint8_t bus, uint8_t slot);
static uint32_t pcnet_io_base;

uint32_t rx_ring_phys;
uint32_t tx_ring_phys;
uint32_t init_block_phys;
uint32_t * rx_ring_virt;
uint32_t * tx_ring_virt;
uint32_t * init_block_virt;

void dma_allocator(dma_region_t *ptr){
    ptr->phys = pmm_alloc_block();
}
static uint16_t pcnet_read_bcr(uint16_t bcr){
    outw(pcnet_io_base+0x12,bcr);
    return inw(pcnet_io_base+0x16);
}
static uint16_t pcnet_read_csr(uint16_t csr)
{
    outw(pcnet_io_base + 0x12, csr);   // RAP
    return inw(pcnet_io_base + 0x10);  // RDP
}
static void pcnet_write_bcr(uint16_t bcr, uint16_t value)
{
    outw(pcnet_io_base + 0x12, bcr);   // RAP
    outw(pcnet_io_base + 0x16, value); // BDP
}

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
                    terminal_write("\n");
                    pcnet_write_bcr(20, 0x0102);

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
    pcnet_io_base = actual_io_port;
    if (!(io_base & 1)) {
        terminal_write("PCnet BAR0 is not an I/O BAR!\n");
        return;
    }

    terminal_write("AMD PCnet I/O Base Address: ");
    terminal_write_hex(actual_io_port);
    terminal_write("\n");
    uint16_t command = pciConfigReadWord(bus, slot, 0, 0x04);
    command |= (1 << 0); // I/O Space Enable
    command |= (1 << 2); // Bus Master Enable
    pciConfigWriteword(bus, slot, 0, 0x04, command);

    uint16_t reset = inw(actual_io_port + 0x14);
    terminal_write("AMD PCnet Reset Register Value: ");
    terminal_write_hex(reset);
    terminal_write("\n");
    uint16_t csr0 = pcnet_read_csr(0);
    terminal_write("AMD PCnet CSR0 Register Value: ");
    terminal_write_hex(csr0);
    terminal_write("\n");
    terminal_write_hex(csr0);

    pcnet_write_bcr(20, 0x0102);
    uint16_t style = pcnet_read_bcr(20);
    terminal_write("BCR20: ");
    terminal_write_hex(style);
    terminal_write("\n");

    if ((style & 0xFF) == 0x02 &&
        (style & 0x0300) == 0x0300) {
        terminal_write("PCnet-32 software style verified!\n");
    } else {
        terminal_write("PCnet software style verification failed!\n");
    }
}

void pcnet_init(void)
{
    terminal_write("Initializing AMD PCnet driver...\n");
    pcnet_scan();
}
