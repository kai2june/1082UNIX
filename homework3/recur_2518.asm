_start:
    push rbp
    mov rbp, rsp
    mov eax, 5
    call formula
    pop rbp
    jmp exit
non_positive:
    mov eax, 0
    ret
it_is_one:
    mov eax, 1
    ret
formula:
    ; push rbp
    ; mov rbp, rsp
    ; push rbx
    ; push rcx
    ; push rdx

    cmp eax, 0
    jle non_positive
    cmp eax, 1
    je it_is_one

    mov edx, eax
    sub eax, 1
    call formula
    mov ebx, eax
    inc ebx
    mul ebx
    mov ebx, eax

    mov eax, edx
    sub eax, 2
    call formula
    mov ecx, eax
    inc ecx
    mul ecx
    mov ecx, eax

    add ebx, ebx ; ebx *= 2
    mov edx, ecx ; ecx *= 3
    add ecx, ecx ; ecx *= 3
    add ecx, edx ; ecx *= 3
    
    mov eax, ebx
    add eax, ecx

    ; pop rdx
    ; pop rcx
    ; pop rbx
    ; mov rsp, rbp
    ; pop rbp
    ret
exit:
    done: