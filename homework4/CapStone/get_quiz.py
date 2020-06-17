from pwn import *
import binascii as b
import numpy as np

c = remote("aup.zoolab.org", 2530)

'''
recvuntil(">>> ") will sometimes cause EOFError
try more time and we'll get the flag
'''
for i in range(10): # 10 quiz
    c.recvuntil(">>> ") # receive until >>>
    g = c.recvuntil("\n") # receive byte question
    st =  g[:-1] # eliminate \n
    by = b.a2b_hex(st) # convert ascii to byte

    context.arch = 'x86_64'
    dis = disasm(by, byte=False, offset=False) # byte and offset column will not be showed

    # parse to get desired format
    rlt = dis[0]
    for idx in range(1, len(dis)):
        if dis[idx] == ' ' and ( ( dis[idx-1] == ' ' ) ):
            pass
        else:
            rlt += dis[idx]

    # answer
    print(b.b2a_hex(bytes(rlt, 'utf-8')))
    c.sendline(b.b2a_hex(bytes(rlt, 'utf-8')))

# receive FLAG
c.recvuntil("ASM")
ans = c.recvline()
print(ans)

