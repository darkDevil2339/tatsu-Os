#ifndef AMD_PCNET_H
#define AMD_PCNET_H
#include <stdint.h>
uint32_t io_base; // I/O base address for the AMD PCnet device
#define PCNET_RDP   (io_base + 0x10)
#define PCNET_RAP   (io_base + 0x12)
#define PCNET_RESET (io_base + 0x14)
#define PCNET_BDP   (io_base + 0x16)
void pcnet_scan(void);
void pcnet_init(void);
void pcnet_init_device(uint8_t bus, uint8_t slot);
#endif // AMD_PCNET_H