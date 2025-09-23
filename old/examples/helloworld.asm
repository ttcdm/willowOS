[org 0x7C00]

;teletype mode

mov ah, 0x0e

mov bx, x


other:
	mov al, [bx];this is because we need an external variable to increment it. it's like when you write a while loop and you need to initialize a variable outside of the loop in order to stop it because if you intialize it inside the loop it just starts over and over again
	cmp al, 0x0
	je exit
	int 0x10
	inc bx
	jmp other
	
exit:
	jmp $
x:
	db "hellworld", 0

jmp $; $ is current line and $$ is current sector or something

times 510-($-$$) db 0
db 0x55, 0xaa

