//
// Created by Zixiong Liu on 12/17/16.
//

#ifndef MYOS_INTERRUPTS_H
#define MYOS_INTERRUPTS_H


/* Defines an IDT entry */
struct idt_entry
{
    uint16_t offset_1; // offset bits 0..15
    uint16_t selector; // a code segment selector in GDT or LDT
    uint8_t ist;       // bits 0..2 holds Interrupt Stack Table offset, rest of bits zero.
    uint8_t type_attr; // type and attributes
    uint16_t offset_2; // offset bits 16..31
    uint32_t offset_3; // offset bits 32..63
    uint32_t zero;     // reserved
    static idt_entry make_gate(void*);
} __attribute__((packed));

struct idt_ptr
{
    uint16_t limit;
    uint64_t base;
    void install();
} __attribute__((packed));

struct interrupt_frame
{
    uint64_t rax, rbx, rcx, rdx;
    uint64_t rdi, rsi, rsp, rbp;
    uint64_t r8,  r9,  r10, r11;
    uint64_t r12, r13, r14, r15;
    uint64_t flags;
} __attribute__((packed));

extern struct idt_ptr idtp;
#endif //MYOS_INTERRUPTS_H
