extern test_a
extern usermode_stack_base
extern syscall_switcher


[global jump_to_user]
jump_to_user:
    mov rcx, test_a
    pushfq
    pop r11
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

    mov rdi, rax
    call syscall_switcher

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