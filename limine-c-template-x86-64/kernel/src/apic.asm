;https://osdev.wiki/wiki/Symmetric_Multiprocessing
; This code will be relocated to 0x8000, sets up environment for calling a C function
bits 16
section .text
extern stack_top
extern bspdone
extern aprunning
extern ap_startup
[global ap_trampoline]
ap_trampoline:
    cli
    cld
    jmp 0x0:relocate       ; Far jump to 0x8040

align 16
_L8010_GDT_table:
    dq 0x0000000000000000          ; Null Descriptor
    dq 0x00CF9A000000FFFF          ; Flat Code Segment Descriptor (0x08)
    dq 0x00CF92000000FFFF          ; Flat Data Segment Descriptor (0x10)
    dq 0x00CF890000006800          ; TSS Descriptor (Unused)

_L8030_GDT_value:
    dw _L8030_GDT_value - _L8010_GDT_table - 1
    dd _L8010_GDT_table
    dd 0
    dd 0

align 64
relocate:
    xor ax, ax
    mov ds, ax
    lgdt [_L8030_GDT_value]          ; Load GDT

    mov eax, cr0
    or eax, 0x1                      ; Set PE (Protected Mode Enable) bit
    mov cr0, eax

    jmp 0x08:protected_mode_entry     ; Far jump to flush prefetch queue

bits 32
protected_mode_entry:
    mov ax, 0x10
    mov ds, ax
    mov ss, ax

    ; Get Local APIC ID
    mov eax, 1
    cpuid
    shr ebx, 24
    mov edi, ebx

    ; Set up 32k stack for each core
    shl ebx, 15
    mov eax, stack_top
    sub eax, ebx
    mov esp, eax

    push edi

wait_bsp:
    pause
    cmp byte [bspdone], 0
    je wait_bsp

    ; Mark this AP as running
    lock inc byte [aprunning]

    ; Jump to C code (Long Mode)
    jmp 0x08:ap_startup
