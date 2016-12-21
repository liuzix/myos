//
// Created by Zixiong Liu on 12/16/16.
//
#include <stdint.h>
#include <cstddef>
#include "lib/printf.h"
#include "lib/assert.h"
#include "frame.h"
#include "paging.h"
#include "heap_allocator.h"

inline uint8_t* get_al8(uint8_t* addr) {
  uint64_t addr_int = (uint64_t)addr;
  if (addr_int % 8 != 0) addr_int = (addr_int / 8) * 8 + 8;
  return (uint8_t*) addr_int;
}

struct multiboot_tag {
    uint32_t type;
    uint32_t size;
    uint64_t data[]; // data plus padding
};

struct multiboot_info {
    size_t total_size;
    multiboot_tag* tags[20]; // assume less than 20 tags

    void parse(void* ptr) {
      total_size = *(uint32_t*)ptr;
      printf("total_size = %d\n", total_size);
      assert_true( *((uint32_t*)ptr + 1) == 0);
      multiboot_tag* iter = (multiboot_tag*)(get_al8((uint8_t *)ptr + 8));
      iter = (multiboot_tag*)(get_al8((uint8_t *)iter));
      int sum = 0;
      for (int i = 0; i < 20; i++) {
        tags[i] = iter;
        sum += iter->size;
        if (sum > total_size) {
          printf("sanity check failed!\n");
          return;
        }
        if (iter->type == 0 && iter->size == 8) {
          printf("Num of tags = %d\n", i);
          return;
        }
        printf("tag type = %d, size = %d\n", iter->type, iter->size);
        iter = (multiboot_tag*)((uint8_t *)iter + iter->size);
        iter = (multiboot_tag*)(get_al8((uint8_t *)iter));
      }
      printf("You reached somewhere unreachable! too many tags!\n");
    }

    multiboot_tag* find_tag(uint32_t type) {
      for (int i = 0; i < 20; i++) {
        if (tags[i]->type == 0 && tags[i]->size == 8) {
          return nullptr; // not found
        }
        if (tags[i]->type == type) {
          return tags[i];
        }
      }
      return nullptr;
    }
};

struct mem_entry {
    uint8_t* base;
    size_t length;
    uint32_t type;
    int32_t reserved;
};

struct mem_region_iter {
    mem_entry* cur_entry;
    size_t offset;
    uint32_t total;
    mem_region_iter (multiboot_tag* mem_tag) {
      assert_true(mem_tag->type == 6);
      cur_entry = (mem_entry*)(&mem_tag->type + 4);
      offset = 4 * 4; // skip four fields in tag
      total = mem_tag->size;
      printf("entry len = %u\n", total);
    }

    mem_entry get_entry () {
      return *cur_entry;
    }

    mem_entry next_entry() {
      offset += 24;
      if (offset >= total) {
        mem_entry dummy;
        dummy.type = 0xdeadbeef; // means no more regions
        return dummy;
      }
      else {
        //printf("size of mem entry %d\n", sizeof(mem_entry));
        cur_entry++;
        //assert_true(cur_entry->reserved == 0);
        offset += 24;
        return *cur_entry;
      }
    }
};

#define MEM_TAG_NUM 6
#define FR_MNGR_SIZE_PG 16
#define KERNEL_OFFSET ( 100 * 1024 * 1024 )
//max kernel size = 5M

extern frame* frame_manager;

void meminit (void* boot_info) {
  multiboot_info info;
  info.parse(boot_info);
  auto mem_tag = info.find_tag(MEM_TAG_NUM);
  mem_region_iter iter(mem_tag);
  auto this_region = iter.get_entry();
  uint8_t * mem_addr;
  size_t mem_size = 0;
  while (this_region.type != 0xdeadbeef) {
    if (this_region.type == 1) {
      mem_addr = this_region.base;
      mem_size = this_region.length;
      printf("Mem region addr = %08x, size = %lu\n", this_region.base, this_region.length);
    }

    this_region = iter.next_entry();
  }
  mem_addr += KERNEL_OFFSET;
  frame* fr_manager = ::new(mem_addr) frame(FR_MNGR_SIZE_PG * PAGE_SIZE, mem_addr);
  fr_manager->set_pool(mem_addr + FR_MNGR_SIZE_PG * PAGE_SIZE, (mem_size / PAGE_SIZE) - FR_MNGR_SIZE_PG);
  frame_manager = fr_manager;

  page_init();


  /// testing
  pages.map((uint8_t *)0xb0007000, frame_manager->alloc_sys());
  printf("testing paging...\n");
  *(int*)(0xb0007000) = 1234;
}