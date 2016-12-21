
extern intr_handler

section .text

do_intr:
    ;mov rbp, rsp
    ;mov rbp, rsp
    pushfq
    push r15
    push r14
    push r13
    push r12
    push r11
    push r10
    push r9
    push r8
    push rbp
    push rsp
    push rsi
    push rdi
    push rdx
    push rcx
    push rbx
    push rax

    mov rdi, [temp]
    mov rsi, rsp
    call intr_handler


    pop rax
    pop rbx
    pop rcx
    pop rdx
    pop rdi
    pop rsi
    add rsp, 8
    pop rbp
    pop r8
    pop r9
    pop r10
    pop r11
    pop r12
    pop r13
    pop r14
    pop r15
    popfq
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