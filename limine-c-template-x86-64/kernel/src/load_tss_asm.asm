[global load_tss_asm]
load_tss_asm:
	mov ax, 0x28  ;The descriptor of the TSS in the GDT (e.g. 0x28 if the sixths entry in your GDT describes your TSS)
	ltr ax        ;The actual load
