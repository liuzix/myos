//
// Created by Zixiong Liu on 12/20/16.
//

#include "ahci.h"
#include "../lib/printf.h"
#include "../paging.h"
#include "pci.h"
#include "../frame.h"
#include "../thread.h"
#include "../utils.h"
#include <vector>
#include <string.h>

uint64_t ahci_base;

#define MAX_NUM_HDDS 6
block_device* hdds[6];
static int num_devices = 0;


uint32_t ahci_get_reg(uint16_t offset);
void ahci_set_reg(uint16_t offset, uint32_t val);
void set_up_device(uint16_t port);

void set_working_status(block_device* device, bool turn_on) ;

void set_up_receive_buf(block_device* device) ;

void ahci_set_irq(int vec) ;

void ahci_init_controller (pci_device* device) {
  kprintf("starting ahci initialization\n");
  uint16_t low_half = pci_read_word(device->bus, device->slot, device->func, 0x24);
  uint16_t high_half = pci_read_word(device->bus, device->slot, device->func, 0x26);
  //printf("ahci base address = %lx\n", low_half);
  uint32_t whole = ((uint32_t)(high_half) << 16) | low_half;
  uint64_t addr = 0;
  addr = whole >> 12 << 12;
  kprintf("ahci base address = %lx\n", addr);
  ahci_base = addr;
  auto p = pages.map((uint8_t *)ahci_base, (uint8_t *)ahci_base);
  p->cache_disabled = true;
  p->writable = true;
  ahci_set_reg(0x4, (uint32_t)1 << 31);
  uint32_t ports = ahci_get_reg(0x0c);
  kprintf("ports implemented %08x\n", ports);



  //enable the device
  //pci_write_word(33 ,device->bus, device->slot, device->func, 0x3c); //interrupt vector 33
  pci_write_word(0b01101010111,device->bus, device->slot, device->func, 0x4);
  //kprintf("pci interrupt status %d\n", pci_read_word(device->bus, device->slot, device->func, 0x3c));
  int pci_irq = pci_read_word(device->bus, device->slot, device->func, 0x3c);
  pci_irq &= 0xff;
  ahci_set_irq(pci_irq);
  kprintf("AHCI using irq 0x%x\n", pci_irq);

  ahci_set_reg(0x8, 1);
  ahci_set_reg(0x4, (1 << 31) | 0b10); // AE = 1 IE = 1

  kprintf("HBA Global %x\n", ahci_get_reg(04));

  memset(hdds, 0, sizeof(block_device*) * MAX_NUM_HDDS);
  for (int i = 0; i < 32; i++) { // iterate over ports
    bool port_available;
    port_available = (bool)((ports >> i) & 1);
    if (port_available) {
      uint16_t port_reg_off = 0x100 + 0x80 * i;
      uint32_t status = ahci_get_reg(port_reg_off + 0x28);
      if ((status & 0b111) == 3) {
        kprintf("SATA device on port %d\n", i);
        set_up_device(i);
      }
    }
  }
}


void set_up_device(uint16_t port) {


  block_device *device = new block_device;
  device->port = port;
  device->regbase = 0x100 + 0x80 * port;
  hdds[num_devices++] = device;
  uint32_t old_status = ahci_get_reg(device->regbase + 0x18);

  //while ((ahci_get_reg(device.regbase + 0x18) >> 14) & 0b11);
  uint32_t intra_status = ahci_get_reg(device->regbase + 0x18);
  kprintf("device running status %x\n", old_status);

  set_working_status(device, false);

  device->command_list = (HBA_CMD_HEADER*) frame_manager->alloc_sys();
  kprintf("device signature %x\n", ahci_get_reg(device->regbase + 0x24));
  ahci_set_reg(device->regbase + 0x0, (uint64_t )device->command_list );
  ahci_set_reg(device->regbase + 0x4, ((uint64_t )device->command_list >> 32));

  device->received_fis = (HBA_FIS* ) frame_manager->alloc_sys();
  ahci_set_reg(device->regbase + 0x8, (uint64_t )device->received_fis );
  ahci_set_reg(device->regbase + 0xc, ((uint64_t )device->received_fis >> 32));

  device->command_table = (HBA_CMD_TBL*)  frame_manager->alloc_sys();
  device->command_list->ctba = (uint64_t) device->command_table;
  device->command_list->ctbau = (uint64_t) device->command_table >> 32;
  device->command_list->prdtl = 1;
  device->command_list->cfl = sizeof(FIS_REG_H2D) / sizeof(DWORD);
  set_up_receive_buf(device);
  ahci_set_reg(device->regbase + 0x10, -1);
  ahci_set_reg(device->regbase + 0x14, ~0);
  //device.command_table->prdt_entry->i = 1;
  // try to identify driver
  kprintf("device running status %x\n", ahci_get_reg(device->regbase + 0x18));

  FIS_REG_H2D* send_fis = (FIS_REG_H2D*)device->command_table->cfis;
  send_fis->fis_type = 0x27;
  send_fis->c = 1;
  send_fis->command = 0xEC;
  send_fis->device = 0;

  uint32_t old_cmd = ahci_get_reg(device->regbase + 0x18);

  device->received_fis->rfis.fis_type = 0;
  set_working_status(device, true);
  kprintf("device interrupt status %x\n", ahci_get_reg(device->regbase + 0x10));
  thread_current->thread_sleep(100);
  kprintf("device identify issuing... \n");
  ahci_set_reg(0x8, -1);
  ahci_set_reg(device->regbase + 0x10, -1);
  ahci_set_reg(device->regbase + 0x14, ~0);
  ahci_set_reg(device->regbase + 0x34, 1); // indicate command slot available
  ahci_set_reg(device->regbase + 0x38, 1); // indicate command slot available
  device->wait_for();
  kprintf("device running status %x\n", ahci_get_reg(device->regbase + 0x18));
  thread_current->thread_sleep(100);
  kprintf("device interrupt status %x\n", ahci_get_reg(device->regbase + 0x10));

  char model[40];
  memmove(model, device->receive_buf + 54, 40);
  for (int i = 0; i < 40; i+=2) {
    char temp = model[i+1];
    model[i+1] = model[i];
    model[i] = temp;
  }
  model[40] = 0;
  kprintf("model: %s\n", model);
}

void set_working_status(block_device* device, bool turn_on) {
  //turn on & off the device
  uint32_t old_status = ahci_get_reg(device->regbase + 0x18);
  if (turn_on) {
    ahci_set_reg(device->regbase + 0x18, old_status | (1 << 4)); // FRE = 1, SUD = 1, ST =1
    ahci_set_reg(device->regbase + 0x18, old_status | (1 << 4) | 3);
  } else {
    ahci_set_reg(device->regbase + 0x18, old_status & ~0b1 );
    ahci_set_reg(device->regbase + 0x18, old_status & ~0b10001 ); // stop the device
  }
}

void set_up_receive_buf(block_device* device) {
  auto p = frame_manager->alloc_sys();
  device->command_table->prdt_entry->dba = (uint64_t) p;
  device->command_table->prdt_entry->dbau = ((uint64_t) p )>> 32;
  device->command_table->prdt_entry->dbc = 4095;
  device->command_table->prdt_entry->i = 1;
  device->command_table->prdt_entry->rsv0 = 0;
  device->command_table->prdt_entry->rsv1 = 0;
  device->receive_buf = p;
  memset(device->receive_buf, 0, 4096);
  // setup receive buf enough to hold one block, i.e. 512bytes
}

uint32_t ahci_get_reg(uint16_t offset) {
  uint64_t addr = ahci_base + offset;
  uint32_t res = 0;
  asm("clflush (%1); movl (%1), %0;"
  : "=r"(res) : "r"(addr) :"memory");
  return res;
}

void ahci_set_reg(uint16_t offset, uint32_t val) {
  uint64_t addr = ahci_base + offset;
  asm("movl %0, (%1); clflush (%1);"
  : : "r"(val), "r"(addr) :"memory");
}

void ahci_on_interrupt() {
  kprintf("AHCI interrupt!\n");
  uint32_t intra = ahci_get_reg(0x8); // read interrupt status
  //kprintf("controller interrupt status %x\n", intra);
  for (int i = 0; i < num_devices; i++) {
    if (!hdds[i]) continue; // no such device
    //kprintf("device port %d interrupt status %x\n", iter->port, ahci_get_reg(iter->regbase + 0x10));
    if ((intra >> i) & 1 == 0 ) continue; // no interrupt on this device
    hdds[i]->on_interrupt();
    ahci_set_reg(hdds[i]->regbase + 0x10,  -1);
  }
  ahci_set_reg(0x8, -1); // clear interrupt bit
}

void ahci_set_irq(int vec) {
  int idt_vec = vec + 32;
  sata_int = idt_vec;
}

void block_device::on_interrupt() {
  if (waiting_thread) {
    waiting_thread->unblock();
    kprintf("Interrupt here! wake up thread\n");
  } else {
    outstanding_interrupts++;
  }
}

void block_device::wait_for() {

  if (outstanding_interrupts > 0) {
    outstanding_interrupts--;
    return;
  }
  waiting_thread = thread_current;
  waiting_thread->block();
  waiting_thread->yield();
  return;
}
