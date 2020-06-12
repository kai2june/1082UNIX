lea eax, [edi*2] ; edi必須乘以2的倍數
lea ebx, [eax + edi]
lea ecx, [eax + ebx]
lea edx, [ecx + edi*4] ; edi必須乘以2的倍數
done:
