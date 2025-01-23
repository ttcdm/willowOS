;section .text
;[bits 32]

[global jump_gdt]
jump_gdt:
	ljmp 0xfebc , 0x12345678 
longjmp_after_gdt:
    mov ax, 0x10               ; Load the selector for data segments
    mov ds, ax                 ; Update DS
    mov es, ax                 ; Update ES
    mov fs, ax                 ; Update FS
    mov gs, ax                 ; Update GS
    mov ss, ax                 ; Update SS
    ;mov rsp,  ; Set the stack pointer
    ret