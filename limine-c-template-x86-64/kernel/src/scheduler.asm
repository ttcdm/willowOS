extern swap_to_user_gs
extern get_current_thread



[global push_all_regs];to avoid possible conflict with push_reg macro
push_all_regs:
    push rax
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi
    push rbp
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15
    ret

[global pop_all_regs]
pop_all_regs:
    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rbp
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
    pop rax
    ret


[global switch_thread]
;.type switch_thread, @function
switch_thread:
cli
    ; push rbx
    ; push rbp
    ; push r12
    ; push r13
    ; push r14
    ; push r15

    ; push rax

    push rax
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi
    push rbp
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15

    call get_current_thread
    fxsave [rax + 0x50]
    

    ;call push_all_regs

    ; mov rax, rsp
    mov [rdi], rsp
    mov rsp, rsi

    ;call pop_all_regs

    call get_current_thread
    fxrstor [rax + 0x50]


    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rbp
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
    pop rax
    
    ; pop rax

    ; pop r15
    ; pop r14
    ; pop r13
    ; pop r12
    ; pop rbp
    ; pop rbx

    ; call swap_to_user_gs

    ; call swap_to_user_or_kernel_gs

sti
    ret

