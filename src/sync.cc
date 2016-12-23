//
// Created by Zixiong Liu on 12/19/16.
//

#include "sync.h"
#include "lib/assert.h"
#include "thread.h"

extern "C" void spinlock_lock(uint8_t*);

void spinlock::lock() {
  if (thread_current == nullptr) {
    lock_var = 0;
    return;
  } // no threading
  spinlock_lock(&lock_var);
  if (lock_var != 1) panic();
}

void spinlock::unlock() {
  if (thread_current == nullptr) return; // no threading
  //assert_true(lock_var == 1); // lock already held;
  lock_var = 0;
}

void sleep_lock::lock() {
  if (!initialized) return;
  guard.lock();
  assert_true(held_by != thread_current);
  while (held_by != nullptr) {
    thread_current->block();
    wait_list.push_back(thread_current);
    guard.unlock();
    thread_current->yield();
    // now waked up. loop in case another has held the lock
    guard.lock();
  }
  held_by = thread_current;
  guard.unlock();
}

sleep_lock::sleep_lock() {
  kprintf("sleep_lock init!\n");
  if (!initialized) {
    assert_true(thread_current != nullptr);
    initialized = true;
  }
}

void sleep_lock::unlock() {
  if (!initialized) return;
  guard.lock();
  assert_true(held_by == thread_current);
  held_by = nullptr;
  if (!wait_list.empty()) {
    auto another = wait_list.front();
    wait_list.pop_front();
    another->unblock();
  }
  guard.unlock();

}

void critical_lock::lock() {
  uint16_t flags;
  __asm ("pushf; cli;"
    "pop %0 ; "
    : "=r"(flags) : :
  );

  assert_true(!held);
  held = true;
  flags &= 0x200;
  intra_old = (bool) flags;
}

void critical_lock::unlock() {
  assert_true(held);
  held = false;

  if (intra_old) {
    asm ("sti");
  }
}
