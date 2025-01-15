;teletype mode

mov ah, 0x0e
mov al, 0x41; i think single and double quotes are the same in x86 asm

test:
	int 0x10; interrupt for teletype
	inc al

	cmp al, 0x7A
	jne test

jmp $; $ is current line and $$ is current sector or something

times 510-($-$$) db 0
db 0x55, 0xaa

