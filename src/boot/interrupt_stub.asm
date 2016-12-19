
extern intr_handler

section .text

do_intr:
    push rbp
    mov rbp, rsp
    push rax
    push rbx
    push rcx
    push rdx
    push rdi
    push rsi
    push rsp
    push rbp
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15
    pushfq
    mov rdi, [temp]
    call intr_handler
    popfq
    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rbp
    add rsp, 8
    pop rsi
    pop rdi
    pop rdx
    pop rcx
    pop rbx
    pop rax
    pop rbp

    iretq

%macro intr_handler_gen 1
section .text
global intr_handler_%1

intr_handler_%1:

    mov dword [temp], %1
    jmp do_intr
%endmacro


%assign i 0 
%rep    256
  intr_handler_gen i
%assign i i+1 
%endrep

SECTION .bss
temp:
    resb 8