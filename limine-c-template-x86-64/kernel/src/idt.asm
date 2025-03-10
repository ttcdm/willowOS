%macro isr_err_stub 1
isr_stub_%+%1:
    call exception_handler
    iretq 
%endmacro
; if writing for 64-bit, use iretq instead
%macro isr_no_err_stub 1
isr_stub_%+%1:
    call exception_handler
    iretq
%endmacro


%macro push_reg 0
    pushfq                  ; Save RFLAGS ; chatgpt put this here. not sure if i need this
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
%endmacro

%macro pop_reg 0
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
    popfq                   ; Restore RFLAGS
%endmacro




extern interrupt_handler_custom;remember to extern the functions
extern exception_handler
isr_no_err_stub 0
isr_no_err_stub 1
isr_no_err_stub 2
isr_no_err_stub 3
isr_no_err_stub 4
isr_no_err_stub 5
isr_no_err_stub 6
isr_no_err_stub 7
isr_err_stub    8
isr_no_err_stub 9
isr_err_stub    10
isr_err_stub    11
isr_err_stub    12
isr_err_stub    13
isr_err_stub    14
isr_no_err_stub 15
isr_no_err_stub 16
isr_err_stub    17
isr_no_err_stub 18
isr_no_err_stub 19
isr_no_err_stub 20
isr_no_err_stub 21
isr_no_err_stub 22
isr_no_err_stub 23
isr_no_err_stub 24
isr_no_err_stub 25
isr_no_err_stub 26
isr_no_err_stub 27
isr_no_err_stub 28
isr_no_err_stub 29
isr_err_stub    30
isr_no_err_stub 31


isr_no_err_stub 32
isr_no_err_stub 33
isr_no_err_stub 34
isr_no_err_stub 35
isr_no_err_stub 36
isr_no_err_stub 37
isr_no_err_stub 38
isr_no_err_stub 39
isr_no_err_stub 40
isr_no_err_stub 41
isr_no_err_stub 42
isr_no_err_stub 43
isr_no_err_stub 44
isr_no_err_stub 45
isr_no_err_stub 46
isr_no_err_stub 47
isr_no_err_stub 48
isr_no_err_stub 49
isr_no_err_stub 50
isr_no_err_stub 51
isr_no_err_stub 52
isr_no_err_stub 53
isr_no_err_stub 54
isr_no_err_stub 55
isr_no_err_stub 56
isr_no_err_stub 57
isr_no_err_stub 58
isr_no_err_stub 59
isr_no_err_stub 60
isr_no_err_stub 61
isr_no_err_stub 62
isr_no_err_stub 63

isr_stub_64:
    cli
    push_reg
    mov rdi, rsp
    call interrupt_handler_custom
    pop_reg
    add rsp, 16
    sti
    iretq

global isr_stub_table
isr_stub_table:
%assign i 0 
%rep    65
    dq isr_stub_%+i ; use DQ instead if targeting 64-bit
%assign i i+1 
%endrep