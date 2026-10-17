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

struct pcnet_rx_desc
{
    uint32_t addr;
    int16_t  length;
    uint16_t status;
    uint32_t msg_length;
    uint32_t reserved;
} __attribute__((packed));

struct pcnet_tx_desc
{
    uint32_t addr;
    int16_t  length;
    uint16_t status;
    uint32_t misc;
    uint32_t reserved;
} __attribute__((packed));

struct pcnet_init_block
{
    uint16_t mode;

    /*
     * Bits:
     *
     * 15-12 = TLEN
     * 11-8  = RLEN
     * 7-0   = reserved
     */

    uint16_t tlen_rlen;

    uint8_t phys_addr[6];

    uint16_t reserved;

    uint32_t filter[2];

    uint32_t rx_ring;
    uint32_t tx_ring;
} __attribute__((packed));

void pcnet_scan(void);
void pcnet_init(void);

void pcnet_init_device(uint8_t bus, uint8_t slot);
#endif // AMD_PCNET_H