mov eax, [0x600000]
sal eax, 4
mov ebx, [0x600000]
sal ebx, 3            
mov ecx, [0x600000]
sal ecx, 1
add eax, ebx
add eax, ecx
mov [0x600004], eax
done: