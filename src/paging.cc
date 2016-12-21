//
// Created by Zixiong Liu on 12/18/16.
//

#include "paging.h"
#include "frame.h"
#include "utils.h"

page_table pages(4, p4_table);

page_table::page_table(int level, page_table_entry *addr) {
  this->level = level;
  if (addr == nullptr) {
    // we want to create new table
    entries = (page_table_entry*)frame_manager->alloc_sys();
    _memset(entries, 0, 4096);
    //printf("allocated page table level = %d, addr = %ld\n", level, entries);
  } else
    this->entries = addr;
}

// if create == true, it will create all intermediate page directories
page_table_entry * page_table::get_pte(uint8_t *vaddr, bool create) {
  uint64_t int_addr = (uint64_t) vaddr;
  int high_bit;
  high_bit = level * 9 + 12;
  int low_bit = high_bit - 9;

  uint64_t index;
  uint64_t mask = ((uint64_t )1 << (high_bit - low_bit)) - 1;
  index = (int_addr >> low_bit) & mask;
  if (level == 1) {
    return &entries[index];
  }

  if (entries[index].huge_page) {
    printf("Huge page encountered!\n");
    return &entries[index];
  }
  if (!create && !entries[index].present) {
    return nullptr;
  }
  page_table next_level(level - 1, (page_table_entry*) (entries[index].address * 4096));
  if (!entries[index].present) {
    entries[index].address = (uint64_t) next_level.entries / 4096;
    entries[index].present = true;
    entries[index].writable = true;
    entries[index].write_through = true;
  }
  return next_level.get_pte(vaddr, create);
}

page_table_entry *page_table::map(uint8_t *vaddr, uint8_t *paddr) {
  //printf("mapping %08lx to %08lx\n", vaddr, paddr);
  assert_true(!pages.is_valid(vaddr));
  auto p = pages.get_pte(vaddr, true);
  p->present = true;
  p->address = (uint64_t )paddr / PAGE_SIZE;
  asm ("mov %%cr3, %%rax; mov %%rax, %%cr3" ::: "rax");
  return p;

}
