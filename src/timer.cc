//
// Created by Zixiong Liu on 12/18/16.
//

#include <cstdint>
#include "timer.h"
#include "paging.h"
#include "lib/printf.h"
#include "thread.h"

#define apic_msr 0x1B

uint64_t  apic_base;
uint64_t tick;
void apic_init() {
  asm("cli");
  uint64_t msr_res = 0;
  // read MSR
  asm ("movl %1, %%ecx;"
       "xor %%edx, %%edx;"
       "xor %%eax, %%eax;"
       "rdmsr;"
       "mov %%eax, %0;"
       "shl $32, %%rdx;"
       "or %%rdx, %0": "=m"(msr_res): "i"(apic_msr) : "ecx", "rax", "edx", "memory");
  printf("apic msr = %lx\n", msr_res);
  apic_base = (msr_res >> 12) << 12;
  printf("apic base = %lx\n", apic_base);
  auto p = pages.map((uint8_t *)apic_base, (uint8_t *)apic_base); // identity map the region
  p->writable = true;
  p->cache_disabled = true;
  p->user = false;
  uint32_t u = apic_get_reg(0x320);
  printf("timer reg = %08x\n", u);
  apic_set_reg(0x380, 600000); // about 10 ms a tick
  apic_set_reg(0x3e0, 3);
  apic_set_reg(0x320, 1 << 17 | 0x20);

}

uint32_t apic_get_reg(uint16_t offset) {
  uint64_t addr = apic_base + offset;
  uint32_t res = 0;
  asm("movl (%1), %0;"
      : "=r"(res) : "r"(addr) :"memory");
  return res;
}

void apic_set_reg(uint16_t offset, uint32_t val) {
  uint64_t addr = apic_base + offset;
  asm("movl %0, (%1);"
  : : "r"(val), "r"(addr) :"memory");
}

void time_calibrate() {

}

void on_timer_interrupt(interrupt_frame *fr) {
  tick++;
  asm("cli");
  apic_set_reg(0xB0, 0); // EOI
  //printf("rip at isr %lx\n", fr->rip);
  thread_current->yield();
  //printf("rip resume %lx\n", fr->rip);
}
