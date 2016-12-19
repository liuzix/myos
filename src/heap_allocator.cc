//
// Created by Zixiong Liu on 12/17/16.
//

#include "heap_allocator.h"
#include "lib/assert.h"
#include "frame.h"
#include "paging.h"
#include <stdint.h>

heap_allocator global_heap;
#define KERN_REMAP 0xc0000000

#define ALIGN32(x)        (size_t)((~31)&((x)+31))
#define ALIGNPG(x)        (size_t)((~4095)&((x)+4095))

heap_block *heap_block::split(size_t first_size) {
  assert_true(this->free);
  if (first_size > this->size) return nullptr;
  heap_block* new_block = (heap_block*)(&this->data + first_size);
  new_block->size = this->size - HEADER_SIZE - first_size;
  new_block->free = true;
  this->size = first_size;
  new_block->next = this->next;
  new_block->prev = this;
  if (new_block->next)
    new_block->next->prev = new_block;
  this->next = new_block;
  new_block->magic = 314159;
  return new_block;
}

heap_block *heap_block::try_merge() {

  if (this->is_adjacent_to(this->next) && this->next->free) {
    merge_two(this, this->next);
  }
  if (this->is_adjacent_to(this->prev) && this->prev->free) {
    merge_two(this->prev, this);
    return this->prev;
  }
  return this;
}

void heap_block::merge_two(heap_block *a, heap_block *b) {
  assert_true(a->free && b->free);
  a->next = b->next;
  if (b->next)
    b->next->prev = a;
  a->size += HEADER_SIZE + b->size;
}

inline bool heap_block::is_adjacent_to(heap_block *x) {
  if (x - this == x->size + HEADER_SIZE) {
    return true;
  }
  if (this - x == this->size + HEADER_SIZE) {
    return true;
  }
  return false;
}


heap_block *heap_block::new_block() {
  heap_block* ret = (heap_block*) frame_manager->alloc_sys();
  ret->size = PAGE_SIZE - HEADER_SIZE;
  ret->free = true;
  ret->magic = 314159;
  return ret;
}


heap_block *heap_allocator::alloc(size_t len) {
  //assert_true(len < 4096); // allocation of multiple conseq. pages not implemented
  bool is_huge = len + HEADER_SIZE > 4096;
  len = ALIGN32(len);
  heap_block* ret = nullptr;
  bool is_again = false;
  alloc_cnt++;
again:
  auto iter = this->head;
  while (iter != nullptr) {
    if (iter->free && iter->size >= len) {
      if (iter->size < len * 2) {
        iter->free = false;
        ret = iter;
        goto end;
      } else {
        ret = iter;
        auto nb = iter->split(len);
        if (ret == tail) tail = nb;
        goto end;
      }
    }
    if (iter->next == nullptr)
      assert_true(iter == tail);
    iter = iter->next;
  }
  {
    auto new_block = is_huge ? global_heap.alloc_huge(len) : heap_block::new_block();
    if (!is_huge) page_cnt ++;
    new_block->prev = tail;
    new_block->next = nullptr;
    if (tail) tail->next = new_block;
    tail = new_block;
    if (!head) head = new_block;
    //assert_true(is_again == false);
    is_again = true;
    goto again;
  }
end:
  if (ret->prev) {
    ret->prev->next = ret->next;
  } else
    assert_true(ret == head);

  if (ret->next) {
    ret->next->prev = ret->prev;
  } else
    assert_true(ret == tail);
  if (ret == head) head = ret->next;
  if (ret == tail) tail = ret->prev;
  ret->free = false;
  return ret;
}

void heap_allocator::free(heap_block *b) {
  assert_true(b->free == false);
  auto iter = this->head;
  while (iter != nullptr) {
    if (iter >= b) break;
    iter = iter->next;
  }
  if (iter != nullptr) { // insert somewhere in the middle
    auto prev = iter->prev;
    if (prev) prev->next = b;
    else assert_true(iter == head);

    b->prev = prev;
    iter->prev = b;
    b->next = iter;
    if (iter == head) head = b;
  } else { // insert at the tail
    if (tail) {
      tail->next = b;
      b->prev = tail;
    }
    tail = b;
    if (!head) head = b;
  }

  b->free = true;
  auto merged = b->try_merge();
  if (b == tail)
    tail = merged;

}

void heap_allocator::print_stats() {
  printf("Number of alloc() calls = %ld\n", alloc_cnt);
  printf("Number of pages used = %ld\n", page_cnt);
  int i = 0;
  auto iter = head;
  while (iter) {
    i++;
    iter = iter->next;
  }

  printf("Number of free blocks = %ld\n", i);
}

heap_block * heap_allocator::alloc_huge(size_t len) {
  if (kernel_remap == 0) kernel_remap = (uint8_t *)KERN_REMAP;
  assert_true(len > 4096 - HEADER_SIZE);
  len += HEADER_SIZE;
  len = ALIGNPG(len);
  int page_need = len / 4096;
  assert_true(page_need > 1);
  auto ret = kernel_remap;
  for (int i= 0; i < page_need; i++) {
    auto frame = frame_manager->alloc_sys();
    auto pte = pages.map(kernel_remap, frame);
    pte->writable = true;
    kernel_remap += 4096;
    page_cnt ++;
  }

  auto blk = (heap_block*)ret;
  blk->free = true;
  blk->size = len;
  return blk;
}
