#include "../include/pci.h"
#include "../include/amd_pcnet.h"
#include "../include/pmm.h"
#include "../include/vmm.h"
#include "../include/kstring.h"

extern void terminal_write(const char *s);
extern void terminal_write_hex(uint32_t n);

int pcnet_init_device(uint8_t bus, uint8_t slot);
static uint32_t pcnet_io_base;

// uint32_t rx_ring_phys;
// uint32_t tx_ring_phys;
// uint32_t init_block_phys;
// uint32_t * rx_ring_virt;
// uint32_t * tx_ring_virt;
// uint32_t * init_block_virt;

static uint32_t dma_next_virt = DMA_VIRT_START;
static dma_region_t pcnet_desc_region;
static pcnet_dma_layout_t *pcnet_layout;
static uint32_t rx_ring_phys;
static uint32_t tx_ring_phys;
static dma_region_t rx_buffers[PCNET_RX_RING_SIZE];
static dma_region_t pcnet_tx_buffer;
static dma_region_t pcnet_init_region;
static struct pcnet_init_block *pcnet_init_b;

uint16_t bcnt = 0xF000 | ((uint16_t)(-1536) & 0x0FFF);

void read_mac(uint8_t mac[6])
{
    for (int i = 0; i < 6; i++)
    {
        mac[i] = inb(pcnet_io_base + i);
    }
}
int dma_alloc_page(dma_region_t *ptr)
{
    if (!ptr)
    {
        return 0;
    }
    if (dma_next_virt > DMA_VIRT_END - DMA_PAGE_SIZE)
        return 0;
    ptr->phys = (uint32_t)pmm_alloc_block();
    if (!ptr->phys)
    {
        return 0;
    }
    vmm_map_page((void *)dma_next_virt, (void *)ptr->phys, VMM_WRITABLE);
    ptr->virt = (void *)dma_next_virt;
    ptr->size = DMA_PAGE_SIZE;
    if (vmm_get_phys(ptr->virt) != ptr->phys)
    {
        vmm_unmap_page(ptr->virt);
        pmm_free_block((void *)ptr->phys);
        ptr->phys = 0;
        ptr->size = 0;
        ptr->virt = NULL;
        return 0;
    }
    dma_next_virt += DMA_PAGE_SIZE;
    return 1;
}
static uint16_t pcnet_read_bcr(uint16_t bcr)
{
    outw(pcnet_io_base + 0x12, bcr);
    return inw(pcnet_io_base + 0x16);
}
static uint16_t pcnet_read_csr(uint16_t csr)
{
    outw(pcnet_io_base + 0x12, csr);  // RAP
    return inw(pcnet_io_base + 0x10); // RDP
}
static void pcnet_write_csr(uint16_t csr, uint16_t value)
{
    outw(pcnet_io_base + 0x12, csr);   // Select CSR
    outw(pcnet_io_base + 0x10, value); // Write CSR data
}
static void pcnet_write_bcr(uint16_t bcr, uint16_t value)
{
    outw(pcnet_io_base + 0x12, bcr);   // RAP
    outw(pcnet_io_base + 0x16, value); // BDP
}
int pcnet_wait_init(void)
{
    for (uint32_t i = 0; i < 1000000; i++)
    {
        uint16_t csr0 = pcnet_read_csr(0);

        if (csr0 & 0x0100)
        {
            return 1;
        }
    }

    return 0;
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

                if (vendor_id == 0x1022 && device_id == 0x2000)
                {
                    terminal_write("AMD PCnet device found at bus ");
                    terminal_write_hex(bus);
                    terminal_write(", slot ");
                    terminal_write_hex(slot);
                    terminal_write("\n");

                    // Initialize the specific hardware instance found
                    pcnet_init_device(bus, slot);
                    terminal_write("\n");
                }
            }
        }
    }
}

int pcnet_init_device(uint8_t bus, uint8_t slot)
{
    // Read BAR0 (Offset 0x10) using the correct coordinates
    uint32_t io_base = pciConfigReadDword(bus, slot, 0, 0x10);

    // Mask out the trailing PCI configuration bit flags
    // Bit 0 specifies I/O space (1) or Memory space (0)
    uint32_t actual_io_port = io_base & ~0x3;
    pcnet_io_base = actual_io_port;
    if (!(io_base & 1))
    {
        terminal_write("PCnet BAR0 is not an I/O BAR!\n");
        return 0;
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
        (style & 0x0300) == 0x0300)
    {
        terminal_write("PCnet-32 software style verified!\n");
    }
    else
    {
        terminal_write("PCnet software style verification failed!\n");
    }
    if (!dma_alloc_page(&pcnet_desc_region))
    {
        terminal_write("failed dma alloc page!");
        return 0;
    }
    pcnet_layout = (pcnet_dma_layout_t *)pcnet_desc_region.virt;
    memset(pcnet_layout, 0, sizeof(pcnet_dma_layout_t));
    rx_ring_phys = pcnet_desc_region.phys;
    tx_ring_phys = pcnet_desc_region.phys + sizeof(pcnet_layout->rx);
    terminal_write("DMA physical: ");
    terminal_write_hex((uint32_t)pcnet_desc_region.phys);
    terminal_write("\n");
    terminal_write("DMA virtual: ");
    terminal_write("\n");
    terminal_write_hex((uint32_t)pcnet_desc_region.virt);
    terminal_write("\n");
    terminal_write("rx physical: ");
    terminal_write_hex((uint32_t)rx_ring_phys);
    terminal_write("\n");
    terminal_write("tx physical: ");
    terminal_write_hex((uint32_t)tx_ring_phys);
    for (uint32_t x = 0; x < PCNET_RX_RING_SIZE; x++)
    {

        if (!dma_alloc_page(&rx_buffers[x]))
        {
            terminal_write("RX buffer allocation failed!");
            return 0;
        }

        pcnet_layout->rx[x].addr = rx_buffers[x].phys;

        uint16_t bcnt =
            0xF000 | ((uint16_t)(-PCNET_RX_BUFFER_SIZE) & 0x0FFF);

        pcnet_layout->rx[x].length = bcnt;
        pcnet_layout->rx[x].msg_length = 0;
        pcnet_layout->rx[x].reserved = 0;

        asm volatile("" ::: "memory");

        pcnet_layout->rx[x].status = PCNET_DESC_OWN;
    }
    if (!dma_alloc_page(&pcnet_tx_buffer))
    {
        terminal_write("TX buffer allocation failed!");
        return 0;
    }
    uint8_t *frame = (uint8_t *)pcnet_tx_buffer.virt;
    for (int i = 0; i < 6; i++)
    {
        frame[i] = 0xFF;
    }
    read_mac(&frame[6]);
    frame[12] = 0x88;
    frame[13] = 0xB5;
    char s[] = "TATSU OS PCNET TEST";
    int it;
    for (int t = 0; t < 19; t++)
    {
        frame[14 + t] = s[t];
        it = 14 + t;
    }
    for (int i = it + 1; i < 60; i++)
    {
        frame[i] = 0x00;
    }
    for (uint32_t i = 0; i < PCNET_TX_RING_SIZE; i++)
    {
        pcnet_layout->tx[i].addr = 0;
        pcnet_layout->tx[i].length = 0;
        pcnet_layout->tx[i].misc = 0;
        pcnet_layout->tx[i].reserved = 0;
        pcnet_layout->tx[i].status = 0;
    }

    if (!dma_alloc_page(&pcnet_init_region))
    {
        terminal_write("Init block DMA allocation failed!");
        // return 0;
    }

    pcnet_init_b = (struct pcnet_init_block *)pcnet_init_region.virt;
    memset(pcnet_init_b, 0, sizeof(struct pcnet_init_block));
    pcnet_init_b->rx_ring = rx_ring_phys;
    pcnet_init_b->tx_ring = tx_ring_phys;
    pcnet_init_b->rlen = 0x40;
    pcnet_init_b->tlen = 0x30;
    pcnet_init_b->mode = 0x0000;
    pcnet_init_b->filter[0] = 0;
    pcnet_init_b->filter[1] = 0;
    pcnet_init_b->reserved = 0;
    read_mac(pcnet_init_b->phys_addr);
    terminal_write("\n\n");
    terminal_write("MAC ADDRESS: ");
    for (int x = 0; x < 6; x++)
    {
        terminal_write_hex(pcnet_init_b->phys_addr[x]);
        if (x != 5)
        {
            terminal_write(":");
        }
    }
    terminal_write("\n");
    terminal_write("Init block physical: ");
    terminal_write_hex(pcnet_init_region.phys);

    terminal_write("\nInit block size: ");
    terminal_write_hex(sizeof(struct pcnet_init_block));
    uint32_t init_phys = pcnet_init_region.phys;
    pcnet_write_csr(1, init_phys & 0xFFFF);
    pcnet_write_csr(2, (init_phys >> 16) & 0xFFFF);
    uint16_t csr1 = pcnet_read_csr(1);
    uint16_t csr2 = pcnet_read_csr(2);

    terminal_write("CSR1: ");
    terminal_write_hex(csr1);

    terminal_write("\nCSR2: ");
    terminal_write_hex(csr2);
    terminal_write("\n");
    pcnet_write_csr(0, 0x0001);
    if (!pcnet_wait_init())
    {
        terminal_write("PCnet initialization timeout!\n");
        // return 0;
    }

    terminal_write("PCnet initialization successful!\n");
    pcnet_write_csr(0, 0x0002);
    uint16_t csr00 = pcnet_read_csr(0);

    terminal_write("CSR0 after START: ");
    terminal_write_hex(csr00);
    terminal_write("\n");
    pcnet_layout->tx[0].addr = pcnet_tx_buffer.phys;
    pcnet_layout->tx[0].length = 0xFFC4;
    pcnet_layout->tx[0].misc = 0;
    pcnet_layout->tx[0].reserved = 0;

    pcnet_layout->tx[0].status = PCNET_TX_STP | PCNET_TX_ENP;

    asm volatile("" ::: "memory");

    pcnet_layout->tx[0].status = PCNET_TX_STP | PCNET_TX_ENP | PCNET_DESC_OWN;
    pcnet_write_csr(0, 0x0008);
    terminal_write("TX demand issued!\n");

    uint32_t timeout = 1000000;

    while (pcnet_layout->tx[0].status & PCNET_DESC_OWN)
    {
        if (--timeout == 0)
        {
            terminal_write("TX TIMEOUT!\n");
            return 0;
        }
    }

    terminal_write("TX descriptor released!\n");

    uint16_t tx_status = pcnet_layout->tx[0].status;

    terminal_write("TX status: ");
    terminal_write_hex(tx_status);
    terminal_write("\n");
    uint16_t tx_csr0 = pcnet_read_csr(0);

    terminal_write("CSR0 after TX: ");
    terminal_write_hex(tx_csr0);
    terminal_write("\n");
    return 1;
}

void pcnet_init(void)
{
    terminal_write("Initializing AMD PCnet driver...\n");
    _Static_assert(sizeof(struct pcnet_init_block) == 28, "failed");
    _Static_assert(sizeof(struct pcnet_rx_desc) == 16, "RX descriptor must be 16 bytes");
    _Static_assert(sizeof(struct pcnet_tx_desc) == 16, "TX descriptor must be 16 bytes");
    _Static_assert(_Alignof(struct pcnet_rx_desc) == 16, "RX descriptor alignment error");
    _Static_assert(_Alignof(struct pcnet_tx_desc) == 16, "TX descriptor alignment error");
    _Static_assert(sizeof(pcnet_dma_layout_t) <= 4096, "doesn't fit");
    pcnet_scan();
}
