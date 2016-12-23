//
// Created by Zixiong Liu on 12/19/16.
//

#include "thread.h"
#include "lib/printf.h"
#include "frame.h"
#include "paging.h"
#include <map>
#include "timer.h"
#include "sync.h"
#include <list>
#include <algorithm>


std::list<thread*, SimpleAllocator<> > *ready_list;
std::map<thread*, uint64_t, std::less<thread*>, SimpleAllocator<std::pair<thread* const , uint64_t>> > *sleep_list;
thread* thread_current;
extern thread idle;


void thread_schedule() {
  asm("cli");
  std::map<thread*, uint64_t, std::less<thread*>, SimpleAllocator<std::pair<thread* const, uint64_t>> >::iterator t = sleep_list->begin();
  while(t != sleep_list->end()) {
    thread* k = t->first;
    if (t->second <= tick) {
      k->unblock();
      sleep_list->erase(t);
      t = sleep_list->begin();
      continue;
    }
    t++;
  }
  assert_true(!ready_list->empty());
  thread* next = ready_list->front();
  assert_true(next);
  ready_list->pop_front();
  next->run();
}

extern "C" void thread_exit_stub(void);

extern "C" void thread_exit_callback(thread* this_ptr, int retval) {
  //kprintf("this = %lx\n", this_ptr);
  this_ptr->thread_dying(retval);
}

__attribute__((noreturn))
void thread::run()  {
  assert_true(magic == thread_magic);
  asm ("cli");
  //ready_list->erase(std::remove(ready_list->begin(), ready_list->end(), this)); // remove from ready list
  //ready_list->remove(this);
  running = true;
  thread_current = this;
  if (!initialized) {
    initialized = true;

    //kprintf("starting...\n");
    asm ("mov %0, %%rsp; " :: "g"(stack_bottom) : );
    asm ("push %0" :: "g"(this) : );
    asm ("push %0; " :: "i"(&thread_exit_stub) : );
    asm ("sti; jmp *%0" :: "m"(func) : );
  } else {
    assert_true(rip < 0x6500000);
    //kprintf("re-entering to %lx\n", rip);
    // restore state before interrupt
    //kprintf("saved rip %lx, rsp %lx\n", rip, rsp);
    asm ("mov %0, %%rsp" :: "g"(rsp) : );
    asm ("sti; jmp *%0" :: "m"(rip) : );
  }
}

uint64_t thread::free_stack;
critical_lock thread_lock;


thread::thread(void* entry_point) {
  //kprintf("constructor called!\n");
  magic = thread_magic;
  thread_lock.lock();
  if (!free_stack)
    free_stack = stack_base;

  initialized = false;
  running = false;
  blocked = false;
  dead = false;
  func = entry_point;
  make_stack(4096 * 4);
  ready_list->push_back(this);
  thread_lock.unlock();
}


void thread::make_stack(size_t size) {
  for (size_t mapped = 0; mapped < size; mapped += 4096) {
    uint8_t *phys = frame_manager->alloc_sys();
    auto p = pages.map((uint8_t *) free_stack, phys);
    //auto p = pages.map((uint8_t *) phys, phys);
    free_stack += 4096;
    //stack_bottom = (uint8_t*) phys;
  }
  stack_bottom = (uint8_t *) free_stack;
  //stack_bottom = (uint8_t*) phys;
  free_stack += 4096; // guard page
}

void thread::yield() {
  assert_true(this);
  assert_true(is_thread(this));
  if (!blocked)
    ready_list->push_back(this);
  thread_current = nullptr;
  running = false;
  // ready to save states
  asm ( "push %%rbp; push %%rbx; push %%r8;"
        "mov %%rsp, %0;"
       "movq $returned, %1;"
        : : "m"(rsp), "m"(rip): "rax");
  thread_schedule();
  asm ("returned: ");
  asm volatile ("popq %%r8; popq %%rbx; popq %%rbp": ::);

  return;
}

void thread::thread_dying(int ret) {
  asm("cli");
  dead = true;
  ready_list->remove(this);
  sleep_list->erase(this);
  thread_schedule();
}

void thread::thread_sleep(int ms) {
  if (ms == 0) return;
  asm ("cli");
  this->block();
  (*sleep_list)[thread_current] = (uint64_t ) (tick + ms/10);
  yield();
}

void thread::unblock() {
  this->blocked = false;
  ready_list->push_front(this);
}

void thread::block() {
  this->blocked = true;

}

thread::~thread() {
  assert_true(dead);
  magic = 0;
}

SimpleAllocator<> scheduler_alloc;
SimpleAllocator<std::pair<thread* const, uint64_t>> sleep_alloc;

void thread_init() {
  ready_list = new std::list<thread*, SimpleAllocator<> >(scheduler_alloc);
  sleep_list = new std::map<thread*, uint64_t, std::less<thread*>, decltype(sleep_alloc)>(sleep_alloc);
  kprintf("initializing threading...\n");
}

void test_threading() {

};