mov ecx, [0x600008]
sub ecx, ebx
mov eax, [0x600000]
mov ebx, [0x600004]
neg ebx
imul ebx
cdq
idiv ecx           
mov [0x600008], eax
done: