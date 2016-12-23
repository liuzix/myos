//
// Created by Zixiong Liu on 12/17/16.
//

#include <cstdint>
#include "interrupts.h"
#include "utils.h"
#include "lib/assert.h"
#include "timer.h"
#include "devices/ahci.h"
#include <boost/preprocessor/repetition/repeat.hpp>

// A struct describing a Task State Segment.
struct tss_entry_struct
{
    uint32_t prev_tss;   // The previous TSS - if we used hardware task switching this would form a linked list.
    uint32_t esp0;       // The stack pointer to load when we change to kernel mode.
    uint32_t ss0;        // The stack segment to load when we change to kernel mode.
    uint32_t esp1;       // everything below here is unusued now..
    uint32_t ss1;
    uint32_t esp2;
    uint32_t ss2;
    uint32_t cr3;
    uint32_t eip;
    uint32_t eflags;
    uint32_t eax;
    uint32_t ecx;
    uint32_t edx;
    uint32_t ebx;
    uint32_t esp;
    uint32_t ebp;
    uint32_t esi;
    uint32_t edi;
    uint32_t es;
    uint32_t cs;
    uint32_t ss;
    uint32_t ds;
    uint32_t fs;
    uint32_t gs;
    uint32_t ldt;
    uint16_t trap;
    uint16_t iomap_base;
} __attribute__((packed));

extern uint64_t tss_entry;

struct gdt_entry
{
    unsigned int limit_low:16;
    unsigned int base_low : 24;
    //attribute byte split into bitfields
    unsigned int accessed :1;
    unsigned int read_write :1; //readable for code, writable for data
    unsigned int conforming_expand_down :1; //conforming for code, expand down for data
    unsigned int code :1; //1 for code, 0 for data
    unsigned int always_1 :1; //should be 1 for everything but TSS and LDT
    unsigned int DPL :2; //priviledge level
    unsigned int present :1;
    //and now into granularity
    unsigned int limit_high :4;
    unsigned int available :1;
    unsigned int always_0 :1; //should always be 0
    unsigned int big :1; //32bit opcodes for code, uint32_t stack for data
    unsigned int gran :1; //1 to use 4k page addressing, 0 for byte addressing
    unsigned int base_high :8;
    void install_tss();
} __attribute__((packed));

extern gdt_entry tss;
//extern uint64_t tss;
extern void* int_stack;

void gdt_entry::install_tss() {
  limit_low = sizeof(tss_entry_struct) &0xFFFF;
  base_low = ((uint64_t)&tss_entry) & 0xFFFFFF;
  accessed = 1;
  read_write = 0;
  conforming_expand_down = 0;
  code = 1;
  always_1 = 0;
  DPL = 3;
  present = 1;
  limit_high = 0xF;
  available = 0;
  always_0 = 0;
  big = 0;
  gran = 0;
  base_high = ((uint64_t) &tss_entry &0xFF0000)>> 24;

  //tss_entry.esp0 = (uint64_t) int_stack;
}
//or __attribute__((packed))




idt_entry idt_entry::make_gate(void *func) {
  idt_entry ret;
  ret.offset_1 = (uint16_t) (0xFFFF & (uint64_t)func);
  ret.offset_2 = (uint16_t) (0xFFFF & ((uint64_t)func >> 16));
  ret.offset_3 = (uint16_t) (0xFFFF & ((uint64_t)func >> 32));
  ret.zero = 0;
  ret.selector = 0x08; // gdt code segment
  ret.type_attr = (1 << 7) | (0 << 5) | 0xE;
  ret.ist = 0;
  return ret;
}



/* Declare an IDT of 256 entries. Although we will only use the
*  first 32 entries in this tutorial, the rest exists as a bit
*  of a trap. If any undefined IDT entry is hit, it normally
*  will cause an "Unhandled Interrupt" exception. Any descriptor
*  for which the 'presence' bit is cleared (0) will generate an
*  "Unhandled Interrupt" exception */
struct idt_entry idt[256];
struct idt_ptr idtp;

void default_interrupt_handler(interrupt_frame fr) {
  kprintf("There has been an exception!\n");
  //panic();
}

#define define_handler(z, n, text) extern "C" void intr_handler_ ## n (void);
BOOST_PP_REPEAT(255, define_handler, IGN)

int sata_int = 256;

extern "C" void intr_handler(uint64_t vec_no, interrupt_frame* fr) {
  switch (vec_no) {
    case 32: // timer
      on_timer_interrupt(fr);
      break;

    default:
      if (vec_no == sata_int) {
        ahci_on_interrupt();
        outb(0x20, 0x20);
        break;
      }
      kprintf("Interrupt Number %ld\n", vec_no);
      kprintf("rsp = %lx\n", fr->rsp);
      kprintf("rip = %lx\n", fr->rip);
      panic();
  }

}

#define define_gate(z, n, text) idt[n] = idt_entry::make_gate((void*)&intr_handler_ ## n);

void idt_ptr::install() {

  BOOST_PP_REPEAT(255, define_gate, IGN)
  //0x65154c3
  tss.install_tss();
  idtp.base = reinterpret_cast<uint64_t>(idt);
  idtp.limit = 256 * sizeof(idt_entry);
  asm ("lidt %0" : : "m"(idtp) );
}

