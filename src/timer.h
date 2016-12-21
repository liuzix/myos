//
// Created by Zixiong Liu on 12/18/16.
//
#include "interrupts.h"
#ifndef MYOS_TIMER_H
#define MYOS_TIMER_H



extern uint64_t  apic_base;

void apic_init();
uint32_t apic_get_reg(uint16_t offset);
void apic_set_reg(uint16_t offset, uint32_t val);

void on_timer_interrupt(interrupt_frame* fr);

extern uint64_t tick;



#endif //MYOS_TIMER_H
