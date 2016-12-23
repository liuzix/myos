//
// Created by Zixiong Liu on 12/20/16.
//

#ifndef MYOS_PCI_H
#define MYOS_PCI_H

#include "stdint.h"
struct __pci_driver;

typedef struct {
    uint32_t vendor;
    uint32_t device;
    uint32_t func;
    uint16_t bus;
    uint16_t slot;
    struct __pci_driver *driver;
} pci_device;

typedef struct {
    uint32_t vendor;
    uint32_t device;
    uint32_t func;
} pci_device_id;

typedef struct __pci_driver {
    pci_device_id *table;
    char *name;
    uint8_t (*init_one)(pci_device *);
    uint8_t (*init_driver)(void);
    uint8_t (*exit_driver)(void);
} pci_driver;

extern void pci_init();
extern void pci_proc_dump(uint8_t *buffer);

uint16_t pci_read_word(uint16_t bus, uint16_t slot, uint16_t func, uint16_t offset);
void pci_write_word(uint16_t val, uint16_t bus, uint16_t slot, uint16_t func, uint16_t offset);
#endif //MYOS_PCI_H
