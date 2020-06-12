mov eax, [0x600000]
neg eax
mov ebx, [0x600004]
imul eax, ebx
mov ecx, [0x600008]
add eax, ecx
done: