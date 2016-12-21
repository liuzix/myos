//
// Created by Zixiong Liu on 12/19/16.
//

#ifndef MYOS_THREAD_H
#define MYOS_THREAD_H

#include "interrupts.h"
#include <boost/container/list.hpp>
#define stack_base 0xf00000000
#define thread_magic 0xdeadbeef
class thread {
    uint8_t* stack_bottom;
    void* func;
    void make_stack(size_t size);


    bool initialized;
    bool running;
    bool blocked;
    bool dead;
    static uint64_t free_stack;
    // saved running states
    uint64_t rip;
    uint64_t rsp;
    //

public:
    uint32_t magic;
    void block();
    void run();
    void yield();
    void unblock();
    void thread_dying(int ret);
    void thread_sleep(int ms);
    thread(void* entry_point);
    ~thread();
};

using namespace boost::container;
//extern list<thread*> *ready_list; // need to call constructor somehow
extern thread* thread_current;

inline bool is_thread(thread* t) {
    return (t->magic == thread_magic);
}

void thread_init(); // to be called after malloc is ready
void test_threading();


#endif //MYOS_THREAD_H
