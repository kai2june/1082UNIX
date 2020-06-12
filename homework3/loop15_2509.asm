mov ecx, 0
L1: 
	cmp ecx, 15
	jge L2
	mov al, [0x600000 + ecx] 
	or al, 0x20
	mov [0x600010 + ecx], al
	inc ecx
	jmp L1
L2:
    done: