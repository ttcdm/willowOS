extern test_a
extern syscall_switcher
extern syscall_user_thread_exit

;ALWAYS REMEMBER TO ADD TO THIS LIST
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
extern syscall10
extern syscall11
extern syscall12
extern syscall13
extern syscall14
extern syscall15
extern syscall16
extern syscall17
extern syscall18

;ALWAYS REMEMBER TO ADD TO THIS LIST
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
    dq syscall10
    dq syscall11
    dq syscall12
    dq syscall13
    dq syscall14
    dq syscall15
    dq syscall16
    dq syscall17
    dq syscall18

; .length: dq ($ - syscall_array) / 8;number of elements in the syscall array


section .text
[global swap_to_user_gs]
swap_to_user_gs:
ret
    mov [gs:8], rsp
    mov rsp, [gs:0]
    swapgs

[global swap_to_kernel_gs]
swap_to_kernel_gs:
ret
    swapgs
    mov [gs:0], rsp
    mov rsp, [gs:8]


[global jump_to_user]
jump_to_user:
    cli
    mov rcx, rdi
    mov r11, 0x202
    mov rsp, rsi;not sure if i'm supposed to have brackets around rsi
    ; sub rsp, 4096;//HERE not sure if we should keep this
    ; sub rsp, 1


    ; HERE REMOVED THE EXIT SYSCALL
    
    
    ; mov qword [rsp], syscall_user_thread_exit

    o64 sysret



;put syscall_handler into LSTAR msr before calling syscall
[global syscall_handler]
syscall_handler:
    cli

    push rax

    push rbx
    push rbp
    push r12
    push r13
    push r14
    push r15

    push r11
    push rcx

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

    ;i used nyaux's syscall handler as inspiration
    call [syscall_array + rdi * 8]
    
    cli

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

    pop rcx
    pop r11

    pop r15
    pop r14
    pop r13
    pop r12
    pop rbp
    pop rbx

    pop rax

    o64 sysret