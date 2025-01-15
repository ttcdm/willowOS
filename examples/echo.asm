[org 0x7C00]
;must use bx for general data access?
mov bx, var
other:
	mov ah, 0x00;key press mode or something
	int 0x16
	cmp ah, 0x1c
	je mvbx
	call buffer
	
	mov ah, 0x0e
	int 0x10
	jmp other

buffer:;apparently i had to put this under where the call was but idk?
	
	mov [bx], al
	inc bx
	ret


mvbx:

	mov bx, var
	mov ah, 0x0e
	mov al, 0x0a
	int 0x10
	mov al, 0x0d
	int 0x10
	jmp print
print:
	mov al, [bx];this is because we need an external variable to increment it. it's like when you write a while loop and you need to initialize a variable outside of the loop in order to stop it because if you intialize it inside the loop it just starts over and over again
	cmp al, 0x00
	je exit
	int 0x10
	inc bx
	jmp print
	


exit:
	jmp $
var:
	times 10 db 0

jmp $; $ is current line and $$ is current sector or something

times 510-($-$$) db 0
db 0x55, 0xaa

