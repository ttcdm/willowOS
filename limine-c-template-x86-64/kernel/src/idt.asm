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
%endmacro

;HERE remember to extern the functions
extern exception_handler
extern interrupt_handler_custom;remember to extern the functions
extern page_fault_handler
extern gpf_handler
extern apic_tick_handler
extern sleep_handler;maybe this should be a lower priority idk
extern thread_handler
extern thread_interrupter_handler


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
;isr_err_stub    13;we replace this with our own gpf handler
;isr_err_stub    14;we replace this with our own page fault handler
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

;isr_no_err_stub 64
;isr_no_err_stub 65
;isr_no_err_stub 66
;isr_no_err_stub 67
isr_no_err_stub 68
isr_no_err_stub 69
isr_no_err_stub 70
isr_no_err_stub 71
;isr_no_err_stub 72
isr_no_err_stub 73
isr_no_err_stub 74
isr_no_err_stub 75
isr_no_err_stub 76
isr_no_err_stub 77
isr_no_err_stub 78
isr_no_err_stub 79
isr_no_err_stub 80
isr_no_err_stub 81
isr_no_err_stub 82
isr_no_err_stub 83
isr_no_err_stub 84
isr_no_err_stub 85
isr_no_err_stub 86
isr_no_err_stub 87
isr_no_err_stub 88
isr_no_err_stub 89
isr_no_err_stub 90
isr_no_err_stub 91
isr_no_err_stub 92
isr_no_err_stub 93
isr_no_err_stub 94
isr_no_err_stub 95
isr_no_err_stub 96
isr_no_err_stub 97
isr_no_err_stub 98
isr_no_err_stub 99
isr_no_err_stub 100
isr_no_err_stub 101
isr_no_err_stub 102
isr_no_err_stub 103
isr_no_err_stub 104
isr_no_err_stub 105
isr_no_err_stub 106
isr_no_err_stub 107
isr_no_err_stub 108
isr_no_err_stub 109
isr_no_err_stub 110
isr_no_err_stub 111
isr_no_err_stub 112
isr_no_err_stub 113
isr_no_err_stub 114
isr_no_err_stub 115
isr_no_err_stub 116
isr_no_err_stub 117
isr_no_err_stub 118
isr_no_err_stub 119
isr_no_err_stub 120
isr_no_err_stub 121
isr_no_err_stub 122
isr_no_err_stub 123
isr_no_err_stub 124
isr_no_err_stub 125
isr_no_err_stub 126
isr_no_err_stub 127
isr_no_err_stub 128




isr_stub_13:
    push_reg
    mov rdi, rsp
    sub rsp, 8
    call gpf_handler
    add rsp, 8
    pop_reg
    iretq

isr_stub_14:;page fault handler
    push_reg
    mov rdi, rsp
    sub rsp, 8
    call page_fault_handler
    add rsp, 8
    pop_reg
    iretq

isr_stub_64:;pretty sure this works. it doesn't throw an error anymroe. we don't cli nor sti because we want to allow nested interrupts
    push_reg
    mov rdi, rsp
    sub rsp, 8
    call interrupt_handler_custom
    add rsp, 8
    pop_reg
    iretq

isr_stub_65:
    push_reg
    mov rdi, rsp
    sub rsp, 8
    call apic_tick_handler
    add rsp, 8
    pop_reg
    iretq

isr_stub_66:
    push_reg
    mov rdi, rsp
    sub rsp, 8
    call sleep_handler
    add rsp, 8
    pop_reg
    iretq

isr_stub_67:
    push_reg
    mov rdi, rsp
    sub rsp, 8
    call thread_handler
    add rsp, 8
    pop_reg
    iretq

isr_stub_72:
    push_reg
    mov rdi, rsp
    sub rsp, 8
    call thread_interrupter_handler
    add rsp, 8
    pop_reg
    iretq



global isr_stub_table
isr_stub_table:
%assign i 0 
%rep    129;remember to adjust this for additional vectors
    dq isr_stub_%+i ; use DQ instead if targeting 64-bit
%assign i i+1 
%endrep