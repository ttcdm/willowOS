extern test_a
extern usermode_stack_base
extern syscall_switcher

extern syscall0
extern syscall1
extern syscall2
extern syscall3
extern syscall4
extern syscall5
extern syscall6
extern syscall7
extern syscall8
extern syscall9

section .data
syscall_array:
    dq syscall0
    dq syscall1
    dq syscall2
    dq syscall3
    dq syscall4
    dq syscall5
    dq syscall6
    dq syscall7
    dq syscall8
    dq syscall9

; .length: dq ($ - syscall_array) / 8;number of elements in the syscall array


section .text
[global jump_to_user]
jump_to_user:
    cli
    mov rcx, rdi
    mov r11, 0x202
    mov [gs:8], rsp
    mov rsp, [usermode_stack_base]
    mov [gs:0], rsp

    swapgs
    o64 sysret

;put syscall_handler into LSTAR msr before calling syscall
[global syscall_handler]
syscall_handler:
    swapgs
    ;gs[0:8] is user rsp and gs[8:16] is kernel rsp
    mov [gs:0], rsp
    mov rsp, [gs:8]
    ; push_regs

    push rbx
    push rbp
    push r12 
    push r13
    push r14
    push r15

    push r11
    push rcx

    ; mov rdi, rax
    ; call syscall_switcher

    ;i used nyaux's syscall handler as inspiration
    call [syscall_array + rdi * 8]

    pop rcx
    pop r11

    pop r15
    pop r14
    pop r13
    pop r12
    pop rbp
    pop rbx
    

    mov rsp, [gs:0]
    swapgs
    o64 sysret

; [global push_syscall_args]
; push_syscall_args:
;     push rdi
;     push rsi
;     push rdx
;     ; push rcx
;     push r10
;     push r8
;     push r9

; [global pop_syscall_args]
; pop_syscall_args:
;     pop r9
;     pop r8
;     ; pop rcx
;     pop r10
;     pop rdx
;     pop rsi
;     pop rdi


; [global syscall_asm]
; syscall_asm:; remember that the first arg is always the syscall number
;     call push_syscall_args
;     mov rdi, rax
;     syscall