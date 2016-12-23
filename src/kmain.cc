//
// Created by Zixiong Liu on 12/16/16.
//
#include <stdint.h>
#include "vga.h"
#include "lib/printf.h"
#include "frame.h"
#include "utils.h"
#include "timer.h"
#include "interrupts.h"
#include "heap_allocator.h"
#include "thread.h"
#include "devices/pci.h"
#include "string.h"
#include <vector>


#include <boost/container/vector.hpp>

void meminit (void* boot_info);
using namespace boost::container;
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"

extern "C" void _init(); //call global constructors
extern void call_constructors();
void high_kernel_init();
extern void do_multi_syn_malloc (int n);

int thread_worker() {
  int i = 0;

  while (i < 10) {
    kprintf("hahaha %d\n", i);
    i++;
    thread_current->thread_sleep(1000);
  }
  return 2333;
}

void idle_thread() {

  thread init((void*)&high_kernel_init);

  // after threading is enabled, the kmain routine is permanently suspended
  // we can do other kernel jobs in this thread
  //asm volatile ("mov $1, %%rdx; mov $2, %%rbx; mov $3, %%rcx; mov $4, %%r8": "rdx", "rbx", "rcx", "r8": :);
  while (true) {
    asm ("sti; hlt;");
  }
}

thread idle((void*)&idle_thread);


extern "C" int kmain (int boot_info) {
  void* boot_info_temp = (void*)boot_info; // save a copy in case rdi destroyed
  vga::vga_init();
  vga::print("System loading...\n", COLOR_WHITE);
  vga::print("VGA text is working\n", COLOR_BLUE);
  kprintf("kprintf: boot_info = %u \n", boot_info_temp);
  kprintf("calling into meminit...\n");
  asm("cli");
  //sleep();
  meminit(boot_info_temp);
  // new operator becomes available
  idtp.install();
  apic_init();
  inl(0x21);
  outb(0x21, ~(1 << 7 | 1 << 2));  // disable PIC. We use APIC exclusively
  //outb(0x21, 1 << 4 | 1);
  inl(0xA1);
  outb(0xA1, ~(1 << 3));
  //outb(0xA1, 0);

  thread_init();
  //test_threading();

  call_constructors(); // C++ specification requires global constructors be called

  idle.run();
  asm("sti; hlt;"); // enable interrupt
  // When the first interrupt comes, idle thread will be run
  // NOT REACHED
  __builtin_unreachable();
}

void high_kernel_init() {
  vga::print("Lower kernel initialization finished! \n", COLOR_LIGHT_BLUE);
  //malloc_lock.initialized = true;
  vga::print("Higher kernel initialization started... \n", COLOR_LIGHT_BLUE);
  thread fuck((void*)thread_worker);
  do_multi_syn_malloc(10);
  pci_init();

  while (true) {
    asm ("hlt");
  }
  thread_current->thread_sleep(100);
}

#pragma clang diagnostic pop
