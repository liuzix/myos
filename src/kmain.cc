//
// Created by Zixiong Liu on 12/16/16.
//
#include <stdint.h>
#include "vga.h"
#include "lib/printf.h"
#include "frame.h"
#include "utils.h"
#include "interrupts.h"
#include "heap_allocator.h"

#include <boost/container/vector.hpp>

void meminit (void* boot_info);
using namespace boost::container;
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
extern "C" int kmain (int boot_info) {
  void* boot_info_temp = (void*)boot_info; // save a copy in case rdi destroyed
  vga::vga_init();
  vga::print("System loading...\n", COLOR_WHITE);
  vga::print("VGA text is working\n", COLOR_BLUE);
  printf("printf: boot_info = %u \n", boot_info_temp);
  printf("calling into meminit...\n");
  //sleep();
  meminit(boot_info_temp);

  idtp.install();
  //sleep();
  auto page_1 = frame_manager->alloc_sys();
  auto page_2 = frame_manager->alloc_sys();
  assert_true(page_1 != page_2);


  vector<int> test_list;
  for (int i = 0; i < 5000; i++) {
    test_list.push_back(i);
  }

  for (int i : test_list) {
    printf("num = %d\n", i);
  }

  for (int i = 0; i < 5000; i++) {
    test_list.clear();
  }

  global_heap.print_stats();
  while (true)
    asm ("hlt;");
}
#pragma clang diagnostic pop