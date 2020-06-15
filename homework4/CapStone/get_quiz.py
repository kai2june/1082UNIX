from pwn import *
import binascii as b

c = remote("aup.zoolab.org", 2530)

k = c.recvuntil(">>> ")
g = c.recvuntil("\n")
print("g=", g)
st =  g[:-1]
print("st=", st)
by = b.a2b_hex(st)
print(by)

context.arch = 'x86_64'
print("disasm:", disasm(by))

c.interactive()

c.recvline()
c.recvline()