//
// Created by Zixiong Liu on 12/17/16.
//

#include "heap_allocator.h"
#include "lib/assert.h"
#include "frame.h"
#include "paging.h"
//#include "sync.h"
#include <stdint.h>
#include <string.h>

heap_allocator global_heap;
#define KERN_REMAP 0xc0000000

#define ALIGN32(x)        (size_t)((~31)&((x)+31))
#define ALIGNPG(x)        (size_t)((~4095)&((x)+4095))

//critical_lock malloc_lock;
const size_t header_s = HEADER_SIZE;
heap_block *heap_block::split(size_t first_size) {
  assert_true(this->free);
  assert_true(this->next != (heap_block*)0xcccccccccccc);
  assert_true(this->size - HEADER_SIZE - first_size > 0);
  heap_block* new_block = (heap_block*)((uint8_t*)this + HEADER_SIZE + first_size);
  new_block->size = (this->size - HEADER_SIZE - first_size) & (~31);
  new_block->free = true;
  this->size = first_size;
  new_block->next = this->next;
  new_block->prev = this;
  if (new_block->next)
    new_block->next->prev = new_block;
  this->next = new_block;
  new_block->magic = 314159;
  new_block->free = true;
  new_block->prev = nullptr;
  new_block->next = nullptr;
  //malloc_lock->unlock();
  return new_block;
}

heap_block *heap_block::try_merge() {
  //malloc_lock.lock();
  if (!this->next) return this;
  if (this->is_adjacent_to(this->next) && this->next->free) {
    merge_two(this, this->next);
  }
  if (!this->prev) return this;
  if (this->is_adjacent_to(this->prev) && this->prev->free) {
    merge_two(this->prev, this);
    //malloc_lock.unlock();
    return this->prev;
  }
  //malloc_lock.unlock();
  return this;
}

void heap_block::merge_two(heap_block *a, heap_block *b) {
  //malloc_lock.lock();
 // kprintf("merging %lx abd %lx\n", a, b);
  assert_true(a->free && b->free);
  a->next = b->next;
  if (b->next)
    b->next->prev = a;
  assert_true(b->prev == a);
  a->size += (HEADER_SIZE + b->size);
 // malloc_lock.unlock();
}

inline bool heap_block::is_adjacent_to(heap_block *x) {
  if ((uint64_t)x - (uint64_t)this == x->size + HEADER_SIZE) {
    return true;
  }
  if ((uint64_t)this - (uint64_t)x == this->size + HEADER_SIZE) {
    return true;
  }
  return false;
}


heap_block *heap_block::new_block() {
  heap_block* ret = (heap_block*) frame_manager->alloc_sys();
  ret->size = PAGE_SIZE - HEADER_SIZE;
  ret->free = true;
  ret->magic = 314159;
  ret->prev = nullptr;
  ret->next = nullptr;
  return ret;
}


heap_block *heap_allocator::alloc(size_t len) {
  //malloc_lock.lock();
 // kprintf("locked! %d\n", len);
  assert_true((bool)head == (bool)tail);
  bool is_huge = len + HEADER_SIZE > 4096;
  len = ALIGN32(len);
  heap_block* ret = nullptr;
  alloc_cnt++;

  auto iter = this->head;
  while (iter != nullptr) {
    assert_true(iter->free);
    if (iter->size >= len) {
      if (iter->size < len * 2) {
        ret = iter;
        //goto end;
      } else {
        ret = iter;
        auto nb = iter->split(len);
        //kprintf("splitting %lx into itself and %lx\n", ret, nb);
        if (ret == tail) tail = nb;
        //goto end;
      }
      assert_true((bool)head == (bool)tail);
      if (ret->prev) {
        ret->prev->next = ret->next;
      } else {
        assert_true(ret == head);
        head = ret->next;
      }

      if (ret->next) {
        ret->next->prev = ret->prev;
      } else {
        //assert_true(ret == tail);
        tail = ret->prev;
      }

      break;
    }
    iter = iter->next;
  }
  if (!iter) { // no block found
    auto new_block = is_huge ? global_heap.alloc_huge(len) : heap_block::new_block();
    if (!is_huge) page_cnt ++;
    ret = new_block;
  }

  ret->free = false;
  //kprintf("giving out %lx\n", ret);
  ret->set_check();
  //malloc_lock.unlock();
  //kprintf("unlocked! %d\n", len);

  return ret;
}

void heap_allocator::free(heap_block *b) {
  //malloc_lock.lock();
  b->var_check();
  //kprintf("free called\n");
  assert_true(b->magic == 314159);

  assert_true(b->free == false);
  //memset((void*)&b->data, 0xcc, b->size); // set to 0xcc
  b->prev = (heap_block*)0xcccccccccccc;
  b->next = (heap_block*)0xcccccccccccc;
  auto iter = this->head;
  while (iter != nullptr) {
    if ((uint64_t)iter >= (uint64_t)b) break;
    iter = iter->next;
  }

  assert_true(iter != b);
  if (iter != nullptr) { // insert somewhere in the middle
    //kprintf("inserting %lx between %lx and %lx\n", b, iter->prev, iter);
    b->prev = iter->prev;
    b->next = iter;
    if (iter->prev)
      iter->prev->next = b;
    else {
      assert_true(iter == head);
      head = b;
    }
    iter->prev = b;

  } else { // insert at the tail
    //kprintf("inserting %lx after %lx at the end\n", b,  tail);
    b->next = nullptr;
    if (tail) {
      assert_true(head);
      tail->next = b;
      b->prev = tail;
    } else {
      assert_true(!head);
      head = b;
      b->prev = nullptr;
    }

    tail = b;
  }

  b->free = true;
  //if (b != tail) {
  //  auto merged = b->try_merge();
  //  if (!b->next) tail = b;
  //}
  assert_true((bool)head == (bool)tail);
  //malloc_lock.unlock();

}

void heap_allocator::print_stats() {
  kprintf("Number of alloc() calls = %ld\n", alloc_cnt);
  kprintf("Number of pages used = %ld\n", page_cnt);
  int i = 0;
  auto iter = head;
  while (iter) {
    i++;
    iter = iter->next;
  }

  kprintf("Number of free blocks = %ld\n", i);
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
  blk->prev = nullptr;
  blk->next = nullptr;
  blk->size = len - HEADER_SIZE;

  blk->magic = 314159;
  return blk;
}
