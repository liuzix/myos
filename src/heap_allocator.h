//
// Created by Zixiong Liu on 12/17/16.
//

#ifndef MYOS_HEAP_ALLOCATOR_H
#define MYOS_HEAP_ALLOCATOR_H

#include <stddef.h>
#include <stdint.h>
#include "sync.h"

#define HEADER_SIZE ((offsetof(heap_block, data) << 6) >> 6)

struct heap_block {
    int magic = 314159;
    size_t size;
    bool free;
    heap_block* next;
    heap_block* prev;


    heap_block* split(size_t first_size);
    heap_block* try_merge();
    static void merge_two(heap_block* a, heap_block* b);
    static heap_block* new_block();
    inline static heap_block* get_block(void* mem) {
      return (heap_block*)((uint8_t*)mem - HEADER_SIZE);
    }
private:
    bool is_adjacent_to(heap_block* x);
public:
    uint8_t data;

} ;

class heap_allocator {
private:
    heap_block* head, *tail;
    int alloc_cnt = 0;
    int page_cnt = 0;
    uint8_t* kernel_remap = (uint8_t *)0xc0000000;
public:
    heap_block* alloc(size_t len);
    heap_block * alloc_huge(size_t len);
    void print_stats();
    void free(heap_block*);

};

extern heap_allocator global_heap;
#endif //MYOS_HEAP_ALLOCATOR_H
