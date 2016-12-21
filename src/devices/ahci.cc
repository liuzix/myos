//
// Created by Zixiong Liu on 12/20/16.
//

#include "ahci.h"
#include "../lib/printf.h"
#include "../paging.h"
#include "pci.h"

uint64_t ahci_base;

uint32_t ahci_get_reg(uint16_t offset);
void ahci_set_reg(uint16_t offset, uint32_t val);

void ahci_init_controller (pci_device* device) {
  printf("starting ahci initialization\n");
  uint16_t low_half = pci_read_word(device->bus, device->slot, device->func, 0x24);
  uint16_t high_half = pci_read_word(device->bus, device->slot, device->func, 0x26);
  printf("ahci base address = %lx\n", low_half);
  uint32_t whole = ((uint32_t)(high_half) << 16) | low_half;
  uint64_t addr = 0;
  addr = whole >> 12 << 12;
  printf("ahci base address = %lx\n", addr);
  ahci_base = addr;
  auto p = pages.map((uint8_t *)ahci_base, (uint8_t *)ahci_base);
  p->cache_disabled = true;
  p->writable = true;
  ahci_set_reg(0x4, (uint32_t)1 << 31);
  uint32_t ports = ahci_get_reg(0x0);
  printf("ports implemented %08x\n", ports);
}


uint32_t ahci_get_reg(uint16_t offset) {
  uint64_t addr = ahci_base + offset;
  uint32_t res = 0;
  asm("movl (%1), %0;"
  : "=r"(res) : "r"(addr) :"memory");
  return res;
}

void ahci_set_reg(uint16_t offset, uint32_t val) {
  uint64_t addr = ahci_base + offset;
  asm("movl %0, (%1);"
  : : "r"(val), "r"(addr) :"memory");
}