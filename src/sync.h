//
// Created by Zixiong Liu on 12/19/16.
//

#ifndef MYOS_SYNC_H
#define MYOS_SYNC_H


#include <cstdint>
#include "thread.h"
#include "boost/container/list.hpp"

struct spinlock {
    void lock();
    void unlock();
private:
    uint8_t lock_var = 0;
};

class sleep_lock {
public:
    void lock();
    void unlock();
    bool initialized = false;
    sleep_lock();
private:
    spinlock guard;
    thread* held_by;
    boost::container::list<thread*> wait_list;
};

struct critical_lock {
public:
    void lock();
    void unlock();
private:
    bool held;
    bool intra_old;
};

#endif //MYOS_SYNC_H
