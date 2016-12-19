//
// Created by Zixiong Liu on 12/16/16.
//

#ifndef MYOS_UTILS_H
#define MYOS_UTILS_H

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <sched.h>
#include "lib/printf.h"

inline void outb(uint16_t port, uint8_t val) {
  asm volatile ( "outb %0, %1" : : "a"(val), "Nd"(port) );
  return;
}


void inline _memset(void *s, int c, size_t n) {
  memset(s,c,n);
}

inline void sleep() {
  for (int i = 0; i < 20000; i++)
    printf("waiting...\n");
}


#endif //MYOS_UTILS_H
