#ifndef PCI_H
#define PCI_H
#include <stdint.h>
#include "portio.h"
uint16_t pciConfigReadWord(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);
uint16_t pcicheckvendor(uint8_t bus, uint8_t slot);
#endif