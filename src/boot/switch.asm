global thread_exit_stub
global spinlock_lock
extern thread_exit_callback

section .text
bits 64

; stack layout
; | ---- this ---- |
thread_exit_stub:
    mov rsi, rax
    mov rdi, [rsp]
    jmp thread_exit_callback

; rdi = lock_var
spinlock_lock:
    mov ax, 1
    xchg ax, [rdi]
    test ax, ax
    je got_lock ; loop if lock held
    pause
    jmp spinlock_lock
got_lock:
    ret
