mov ecx, 0
mov edx, 0
L1:
    ; requires edx<9
    cmp edx, 9
    jge L2
    ; check whether swap is required
    mov eax, [0x600000 + edx*4]
    mov ebx, [0x600000 + edx*4 + 4]
    cmp eax, ebx
    jae swap
    ; ++edx
    jmp update_for_loop
L2:
    ; requires ecx<9
    cmp ecx, 9
    jge _exit
    ; new round of bubble sort. ++ecx, edx=0
    inc ecx
    mov edx, 0
    jmp L1
swap:
    ; swap element
    xchg eax, ebx
    mov [0x600000 + edx*4], eax
    mov [0x600000 + edx*4 + 4], ebx
    ; ++edx
    jmp update_for_loop
update_for_loop:
    inc edx
    jmp L1
_exit:
    done: