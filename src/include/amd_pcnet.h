#ifndef AMD_PCNET_H
#define AMD_PCNET_H
#include <stdint.h>
// uint32_t io_base; // I/O base address for the AMD PCnet device
#define PCNET_RDP   (io_base + 0x10)
#define PCNET_RAP   (io_base + 0x12)
#define PCNET_RESET (io_base + 0x14)
#define PCNET_BDP   (io_base + 0x16)
#define PCNET_CSR0       0
#define PCNET_CSR1       1
#define PCNET_CSR2       2

#define PCNET_BCR20      20

#define CSR0_INIT        0x0001
#define CSR0_STRT        0x0002
#define CSR0_STOP        0x0004
#define CSR0_TDMD        0x0008
#define CSR0_IDON        0x0100

#define PCNET_RX_COUNT   8
#define PCNET_TX_COUNT   8

#define PCNET_BUFFER_SIZE 1536

#define PCNET_RX_COUNT 8
#define PCNET_TX_COUNT 8
#define RECURSIVE_PT_BASE 0xFFC00000
#define RECURSIVE_PD      0xFFFFF000
#define DMA_VIRT_START 0xD0000000
#define DMA_VIRT_END   0xD1000000
#define DMA_PAGE_SIZE  4096
#define PCNET_RX_RING_SIZE 16
#define PCNET_TX_RING_SIZE 8
#define PCNET_RX_BUFFER_SIZE 1536
#define PCNET_TX_STP  0x0200
#define PCNET_TX_ENP  0x0100
#define PCNET_DESC_OWN 0x8000
#define PCNET_RX_ERR  0x4000
#define PCNET_RX_STP  0x0200
#define PCNET_RX_ENP  0x0100


typedef struct
{
    uint32_t phys;
    void *virt;
    uint32_t size;
} dma_region_t;

struct pcnet_rx_desc {
    uint32_t addr;
    uint16_t length;
    uint16_t status;
    uint32_t msg_length;
    uint32_t reserved;
} __attribute__((packed, aligned(16)));

struct pcnet_tx_desc {
    uint32_t addr;
    uint16_t length;
    uint16_t status;
    uint32_t misc;
    uint32_t reserved;
} __attribute__((packed, aligned(16)));
struct pcnet_init_block
{
    uint16_t mode;

    uint8_t tlen;
    uint8_t rlen;

    uint8_t phys_addr[6];

    uint16_t reserved;

    uint32_t filter[2];

    uint32_t rx_ring;
    uint32_t tx_ring;
} __attribute__((packed));

typedef struct __attribute__((aligned(16))) {
    struct pcnet_rx_desc rx[PCNET_RX_RING_SIZE];
    struct pcnet_tx_desc tx[PCNET_TX_RING_SIZE];
} pcnet_dma_layout_t;

void pcnet_scan(void);
void pcnet_init(void);
int pcnet_init_device(uint8_t bus, uint8_t slot);
int dma_alloc_page(dma_region_t *ptr);
void pcnet_poll_rx(void);
void pcnet_dump_rx_packet(uint32_t index);
#endif // AMD_PCNET_H