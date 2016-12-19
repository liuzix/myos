//
// Created by Zixiong Liu on 12/16/16.
//

#ifndef MYOS_FRAME_H
#define MYOS_FRAME_H

#include <stdint.h>
#include "lib/bitmap.h"
#include "lib/printf.h"
#include "lib/assert.h"

#define PAGE_SIZE (4 * 1024)


class frame {
private:
    bitmap* free_map;
    uint8_t* base;
    size_t pool_size;
    bool pool_available;
public:
    frame (size_t len, uint8_t* first_frame);
    void set_pool(uint8_t* base, size_t page_cnt);
    uint8_t* alloc_sys();

};
extern frame* frame_manager;

#endif //MYOS_FRAME_H
