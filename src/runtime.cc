//
// Created by Zixiong Liu on 12/18/16.
//

// miscellaneous runtime support
#include "heap_allocator.h"
#include <bits/functexcept.h>
#include "lib/assert.h"
#include "sync.h"

extern "C" void *memset(void *s, int c, size_t n);

critical_lock heap_lock;
void* operator new(size_t size) noexcept
{
  heap_block* new_alloc;
  heap_lock.lock();
  new_alloc = global_heap.alloc(size);
  heap_lock.unlock();
  //asm("sti");
  memset(((uint8_t *)new_alloc) + HEADER_SIZE, 0, size);
  return ((uint8_t *)new_alloc) + HEADER_SIZE;
}

void operator delete(void *p) noexcept
{
  heap_block* tofree;
  heap_lock.lock();
  tofree = heap_block::get_block(p);
  heap_lock.unlock();
  global_heap.free(tofree);
  //asm("sti");
}

void* operator new[](size_t size) noexcept
{
  return operator new(size); // Same as regular new
}

void operator delete[](void *p) noexcept
{
  operator delete(p); // Same as regular delete
}

// A function to copy block of 'n' bytes from source
// address 'src' to destination address 'dest'.
extern "C" void memmove(void *dest, void *src, size_t n)
{
  // Typecast src and dest addresses to (char *)
  char *csrc = (char *)src;
  char *cdest = (char *)dest;

  // Create a temporary array to hold data of src
  char *temp = new char[n];

  // Copy data from csrc[] to temp[]
  for (int i=0; i<n; i++)
    temp[i] = csrc[i];

  // Copy data from temp[] to cdest[]
  for (int i=0; i<n; i++)
    cdest[i] = temp[i];

  delete [] temp;
}

void std::__throw_length_error(const char*) {
  panic();
}

void std::__throw_bad_alloc(void) {
  panic();
}


extern "C" void __assert_func(const char* file, int line, const char* func, const char* what) {
  //if (!cond) {
    kprintf("Assertion failed at %s line %d in %s: %s\n", file, line, func, what);
    panic();
  //}
}


extern "C" void *memset(void *s, int c, size_t n) {
  unsigned char* p;
  p = (uint8_t *)s;
  while(n--)
    *p++ = (unsigned char)c;
  return s;
}

extern void (*__init_list)(void);
extern uint64_t __end_init_list;
void call_constructors() {
  auto iter = __init_list;
  while ((uint64_t)iter > 0) {
    iter();
    iter--;
  }
/*
  auto iter = __init_list;
  while ((uint64_t)iter != __end_init_list) {
    iter();
    iter++;
  }
  */
}

extern "C" void __cxa_atexit() {
  return;
}

//extern "C" void __dso_handle() {
//  return;
//}