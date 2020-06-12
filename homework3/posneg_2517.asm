v1:
    cmp eax, 0x0
    ; leftOp: 0x0 ; rightOp: eax
    jl L1
    mov DWORD PTR [0x600000], 0x1
    jmp v2
L1:
    mov DWORD PTR [0x600000], 0xffffffff
    jmp v2
v2:
    cmp ebx, 0x0
    ; leftOp: 0x0 ; rightOp: ebx
    jl L2
    mov DWORD PTR [0x600004], 0x1
    jmp v3
L2:
    mov DWORD PTR [0x600004], 0xffffffff
    jmp v3
v3:
    cmp ecx, 0x0
    ; leftOp: 0x0 ; rightOp: ecx
    jl L3
    mov DWORD PTR [0x600008], 0x1
    jmp v4
L3:
    mov DWORD PTR [0x600008], 0xffffffff
    jmp v4
v4:
    cmp edx, 0x0
    ; leftOp: 0x0 ; rightOp: edx
    jl L4
    mov DWORD PTR [0x60000c], 0x1
    jmp _exit
L4:
    mov DWORD PTR [0x60000c], 0xffffffff
    jmp _exit
_exit:
    done: