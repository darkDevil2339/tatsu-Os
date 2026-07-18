#ifndef ATA_PIO_H
#define ATA_PIO_H
#include <stdint.h>
void ata_wait_busy();
void ata_wait_drq();
void ata_read_sector(uint32_t lba, uint8_t* buffer);
void ata_write_sector(uint32_t lba, uint8_t* buffer);
#endif