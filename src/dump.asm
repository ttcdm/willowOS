;code that's sort of unused but i don't wanna throw it away

mov al, 0x41; i think single and double quotes are the same in x86 asm

test:
	int 0x10; interrupt for teletype
	inc al

	cmp al, 0x7A
	jne test

mov al, 0x0a
int 0x10