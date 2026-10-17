#include "../include/pci.h"
#include "../include/amd_pcnet.h"
#include "../include/pmm.h"

extern void terminal_write(const char* s);
extern void terminal_write_hex(uint32_t n);

// Forward declaration of the internal init step
void pcnet_init_device(uint8_t bus, uint8_t slot);
static uint32_t pcnet_io_base;
static struct pcnet_rx_desc rx_ring[PCNET_RX_COUNT]
    __attribute__((aligned(16)));

static struct pcnet_tx_desc tx_ring[PCNET_TX_COUNT]
    __attribute__((aligned(16)));

static struct pcnet_init_block init_block
    __attribute__((aligned(16)));

uint32_t rx_ring_phys;
uint32_t tx_ring_phys;
uint32_t init_block_phys;
uint32_t * rx_ring_virt;
uint32_t * tx_ring_virt;
uint32_t * init_block_virt;

static void pcnet_write_csr(uint16_t csr, uint16_t value)
{
    outw(pcnet_io_base + 0x12, csr);   // RAP
    outw(pcnet_io_base + 0x10, value); // RDP
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

static uint16_t pcnet_read_bcr(uint16_t bcr)
{
    outw(pcnet_io_base + 0x12, bcr);   // RAP
    return inw(pcnet_io_base + 0x16);  // BDP
}
static void pcnet_setup_init_block(
    uint32_t rx_ring_phys,
    uint32_t tx_ring_phys)
{
    init_block.mode = 0;

    init_block.tlen_rlen =
        (3 << 12) |   // TX = 8 descriptors
        (3 << 8);     // RX = 8 descriptors

    /*
     * Example MAC.
     * Later we should obtain the real MAC from the PCnet/EEPROM.
     */
    init_block.phys_addr[0] = 0x52;
    init_block.phys_addr[1] = 0x54;
    init_block.phys_addr[2] = 0x00;
    init_block.phys_addr[3] = 0x12;
    init_block.phys_addr[4] = 0x34;
    init_block.phys_addr[5] = 0x56;

    init_block.reserved = 0;

    init_block.filter[0] = 0;
    init_block.filter[1] = 0;

    init_block.rx_ring = rx_ring_phys;
    init_block.tx_ring = tx_ring_phys;
    init_block_phys = pmm_alloc_block();
}
static void pcnet_setup_rx_ring(uint32_t rx_buffer_phys[])
{
    for (int i = 0; i < PCNET_RX_COUNT; i++)
    {
        rx_ring[i].addr = rx_buffer_phys[i];

        /*
         * PCnet uses two's-complement buffer length.
         */
        rx_ring[i].length = -PCNET_BUFFER_SIZE;

        /*
         * Give descriptor to PCnet.
         */
        rx_ring[i].status = 0x8000;

        rx_ring[i].msg_length = 0;
        rx_ring[i].reserved = 0;
    }
}
static void pcnet_setup_tx_ring(void)
{
    for (int i = 0; i < PCNET_TX_COUNT; i++)
    {
        tx_ring[i].addr = 0;
        tx_ring[i].length = 0;
        tx_ring[i].status = 0;
        tx_ring[i].misc = 0;
        tx_ring[i].reserved = 0;
    }
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
    uint16_t csr0 = pcnet_read_csr(0);
    terminal_write("AMD PCnet CSR0 Register Value: ");
    terminal_write_hex(csr0);
    terminal_write("\n");
    terminal_write_hex(csr0);
    pcnet_write_bcr(20, 0x0102);
}

void pcnet_init(void)
{
    terminal_write("Initializing AMD PCnet driver...\n");
    pcnet_scan();
}
