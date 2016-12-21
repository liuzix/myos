//
// Created by Zixiong Liu on 12/20/16.
//

#ifndef MYOS_AHCI_H
#define MYOS_AHCI_H

#include "pci.h"

void ahci_init_controller (pci_device* device);
#endif //MYOS_AHCI_H
