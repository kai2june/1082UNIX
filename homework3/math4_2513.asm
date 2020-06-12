mov eax, [0x600004]
neg eax
cdq  
mov ebx, [0x600008]
idiv ebx
mov ecx, edx
mov eax, [0x600000]
imul eax, 5
neg eax
cdq
idiv ecx
mov [0x60000c], eax
done: