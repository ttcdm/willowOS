[org 0x7C00]
;remember to always add the colons
;0x00 is null char and '0' is 0x30
;try to have an unlimited length string for input
;must use bx for general data access?
mov bx, var
mov bp, 0x8000
mov sp, bp
other:
	mov ah, 0x00;key press mode or something (keyboard mode?)
	int 0x16
	
	times 2 push ax
	cmp ah, 0x1c
	je .call_print
	.ret_print:
	
	pop ax;right now i'm just assuming that whatever gets pushed will be popped by the end of those subroutines
	cmp ah, 0x1c
	je .call_clear_var
	.ret_clear_var:
	
	pop ax
	cmp ah, 0x1c
	; jne .call_write_single;write also calls disp for now
	; .ret_write_single:
	jne .disp_char
	.ret_disp_char:

	jmp other

.call_print:
	call print
	jmp .ret_print
.call_clear_var:
	call clear_var
	jmp .ret_clear_var
.call_write_single:
	call write_single
	
	; jmp .disp_char
	; .ret_disp_char:
	
	; jmp .ret_write_single
.disp_char:
	mov ah, 0x0e
	int 0x10
	jmp .ret_disp_char
write_single:;apparently i had to put this under where the call was but idk?
	mov [bx], al
	inc bx
	ret

clear_var:
	mov bx, var
.cv_loop:
	mov al, [bx]
	cmp al, 0x00
	je .end
	mov [bx], byte 0x00
	inc bx
	jmp .cv_loop
.end:
	ret

print:
	mov ah, 0x0e
	
	mov al, 0x0a;newline
	int 0x10
	mov al, 0x0d;carriage return
	int 0x10
	
	mov bx, var;need to reset bx after inc bx from above
.printl:

	mov al, [bx];this is because we need an external variable to increment it. it's like when you write a while loop and you need to initialize a variable outside of the loop in order to stop it because if you intialize it inside the loop it just starts over and over again
	cmp al, 0x00
	je .end
	int 0x10
	inc bx
	jmp .printl
.end:
	mov al, 0x0a;newline
	int 0x10
	mov al, 0x0d;carriage return
	int 0x10
	ret


exit:
	jmp $
var:
	times 10 db 0x00


jmp $; $ is current line and $$ is current sector or something

times 510-($-$$) db 0x00;0 and 0x00 are the same in terms of numbers but not ascii-wise sorta
db 0x55, 0xaa

