extern test_a
extern user_code
extern usermode_stack_base
extern syscall_switcher


[global jump_to_user]
jump_to_user:

    mov rcx, test_a

    pushfq
    pop r11
    ; mov r11, 0x202
    ; mov rsp, [gs:0]
    mov [gs:8], rsp
    mov rsp, [usermode_stack_base]
    mov [gs:0], rsp
    swapgs
    o64 sysret

; jump_to_user1:
;     call test_a

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
    ;get rdi and index into some array of syscalls
    
    ;call corresponding routine via call

    mov rdi, rax
    call syscall_switcher

    ; call syscall0

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


[global syscall0]
syscall0:
    push r11
    push rcx
    call test_b
    ret

test_b:
    jmp test_b
    ret

; syscall1:
;     cli
;     push rbx
;     push rbp
;     push r12
;     push r13
;     push r14
;     push r15
;     push rsp


;     call jump_to_user

;     pop rsp
;     pop r15
;     pop r14
;     pop r13
;     pop r12
;     pop rbp
;     pop rbx
;     sti