extern test_a

[global jump_to_user]
jump_to_user:

    cli
    push rbx
    push rbp
    push r12
    push r13
    push r14
    push r15
    push rsp


    swapgs
    sysret

syscall1:
    cli
    push rbx
    push rbp
    push r12
    push r13
    push r14
    push r15
    push rsp


    call jump_to_user

    pop rsp
    pop r15
    pop r14
    pop r13
    pop r12
    pop rbp
    pop rbx
    sti