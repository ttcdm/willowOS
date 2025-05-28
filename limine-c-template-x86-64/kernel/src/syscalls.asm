extern test_a
extern user_code
extern top


[global jump_to_user]
jump_to_user:



    ; mov r11, 0x202
    ; mov rax, [rel top]   ; Load the value stored in 'top'
    ; mov rsp, rax         ; Set user stack
    ; mov rcx, user_code
    mov rcx, test_a
    ; mov rcx, aa;
    pushfq
    pop r11
    ; mov r11, 0x202
    ; mov rsp, [gs:0]
    mov rsp, [top]
    swapgs
    o64 sysret

; jump_to_user1:
;     call test_a




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