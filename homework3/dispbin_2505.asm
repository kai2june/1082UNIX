mov ecx, 0
handle_ah: ;;;;;;;;;;
    cmp ecx, 8
    jge handle_al
    shl ah, 1 ;;;;;;;;
    jc write_one
    jmp write_zero
handle_al:
    cmp ecx, 16
    jge _exit
    shl al, 1
    jc write_one
    jmp write_zero
write_one:
    mov BYTE PTR [0x600000 + ecx], '1'
    inc ecx
    cmp ecx, 8
    jge handle_al
    jmp handle_ah ;;;;;;;;;
write_zero:
    mov BYTE PTR [0x600000 + ecx], '0'
    inc ecx
    cmp ecx, 8
    jge handle_al
    jmp handle_ah ;;;;;;;;;;
_exit:
    done: