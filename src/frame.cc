//
// Created by Zixiong Liu on 12/16/16.
//

#include "frame.h"
#include "lib/bitmap.h"
#include "lib/assert.h"
#include "utils.h"
#include "sync.h"

frame* frame_manager;
critical_lock fr_lock;

frame::frame(size_t len, uint8_t *first_frame)  {
  pool_available = false;
  kprintf("frame manager = %08x\n", first_frame);

  free_map = new(first_frame + sizeof(frame)) bitmap(len - sizeof(frame));
  kprintf("freemap = %08x\n", free_map);

  // TODO: can remove later

  kprintf("Testing bitmap implementation..\n");

  for (size_t i = 0; i < len; i++) {
    if (i % 9 == 0) {
      free_map->set(i, true);
    }
    else {
      free_map->set(i, false);
    }
  }

  for (size_t i = 0; i < len; i++) {
    //kprintf("i = %d\n", i);
    if (i % 9 == 0) assert_true(free_map->get(i));
    else assert_true(!free_map->get(i));
  }

  free_map->clear();
  kprintf("Test passed\n");

}

uint8_t *frame::alloc_sys() {
  fr_lock.lock();
  auto ret = free_map->scan_and_set();
  if (ret == free_map->capacity) {
    kprintf("OOM! \n"); panic();
  }
  uint8_t * ret1 = ret * PAGE_SIZE + this->base;
  fr_lock.unlock();
  return ret1;
}

void frame::set_pool(uint8_t *base, size_t page_cnt) {
  assert_true(this->pool_available == false);

  this->base = base;
  this->pool_size = page_cnt;
  this->pool_available = true;
  kprintf("Available physical frames = %u, base = %08x\n", page_cnt, base);
}
