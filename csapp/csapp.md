# CSAPP

## datalab

- 实验本身其实没什么难的，难点在于运算符、常数位数以及运算次数限制
- 做到后面的时候感觉相当吃力，参考了网络文章
- 这章比较重要的点还是在IEEE726浮点数那里，了解规格化浮点数、非规格化浮点数的Bias设置的原因（为了比较平滑地过渡）

## bomblab

没难度，直接读代码就好了，注意其实还有个隐藏关

### 前期

- 先运行一下大概了解流程

```txt
root@0435fe0fdd6e:/ctf/work/TianWen/csapp/bomblab/bomb# ./bomb
Welcome to my fiendish little bomb. You have 6 phases with
which to blow yourself up. Have a nice day!
gangan

BOOM!!!
The bomb has blown up.
```

- objdump看一下函数

```sh
root@0435fe0fdd6e:/ctf/work/TianWen/csapp/bomblab/bomb# objdump -j .text -t bomb
bomb:     file format elf64-x86-64

SYMBOL TABLE:
...
0000000000401062 g     F .text  0000000000000092              phase_5
...
0000000000400f43 g     F .text  000000000000008b              phase_3
0000000000400ee0 g     F .text  000000000000001c              phase_1
...
0000000000401242 g     F .text  0000000000000051              secret_phase
...
0000000000400da0 g     F .text  0000000000000137              main
...
000000000040100c g     F .text  0000000000000056              phase_4
00000000004010f4 g     F .text  0000000000000110              phase_6
...
0000000000400efc g     F .text  0000000000000047              phase_2
...
```

- 再看一下main：

```txt

Disassembly of section .text:

0000000000400da0 <main>:
...
  400e37:       48 89 c7                mov    %rax,%rdi
  400e3a:       e8 a1 00 00 00          callq  400ee0 <phase_1>
  400e3f:       e8 80 07 00 00          callq  4015c4 <phase_defused>
  400e44:       bf a8 23 40 00          mov    $0x4023a8,%edi
  400e49:       e8 c2 fc ff ff          callq  400b10 <puts@plt>
  400e4e:       e8 4b 06 00 00          callq  40149e <read_line>
  400e53:       48 89 c7                mov    %rax,%rdi
  400e56:       e8 a1 00 00 00          callq  400efc <phase_2>
  400e5b:       e8 64 07 00 00          callq  4015c4 <phase_defused>
  400e60:       bf ed 22 40 00          mov    $0x4022ed,%edi
  400e65:       e8 a6 fc ff ff          callq  400b10 <puts@plt>
  400e6a:       e8 2f 06 00 00          callq  40149e <read_line>
  400e6f:       48 89 c7                mov    %rax,%rdi
  400e72:       e8 cc 00 00 00          callq  400f43 <phase_3>
  400e77:       e8 48 07 00 00          callq  4015c4 <phase_defused>
  400e7c:       bf 0b 23 40 00          mov    $0x40230b,%edi
  400e81:       e8 8a fc ff ff          callq  400b10 <puts@plt>
  400e86:       e8 13 06 00 00          callq  40149e <read_line>
  400e8b:       48 89 c7                mov    %rax,%rdi
  400e8e:       e8 79 01 00 00          callq  40100c <phase_4>
  400e93:       e8 2c 07 00 00          callq  4015c4 <phase_defused>
  400e98:       bf d8 23 40 00          mov    $0x4023d8,%edi
  400e9d:       e8 6e fc ff ff          callq  400b10 <puts@plt>
  400ea2:       e8 f7 05 00 00          callq  40149e <read_line>
  400ea7:       48 89 c7                mov    %rax,%rdi
  400eaa:       e8 b3 01 00 00          callq  401062 <phase_5>
  400eaf:       e8 10 07 00 00          callq  4015c4 <phase_defused>
  400eb4:       bf 1a 23 40 00          mov    $0x40231a,%edi
  400eb9:       e8 52 fc ff ff          callq  400b10 <puts@plt>
  400ebe:       e8 db 05 00 00          callq  40149e <read_line>
  400ec3:       48 89 c7                mov    %rax,%rdi
  400ec6:       e8 29 02 00 00          callq  4010f4 <phase_6>
  400ecb:       e8 f4 06 00 00          callq  4015c4 <phase_defused>
  400ed0:       b8 00 00 00 00          mov    $0x0,%eax
  400ed5:       5b                      pop    %rbx
  400ed6:       c3                      retq   
```

可以看到main里按顺序调用了phase1 - phase6这几个函数，随便看一下可以看到这几个函数其实就是要一个个过去的关卡。注意到还有个secret_phase隐藏关卡

### phase_1

```txt
Disassembly of section .text:

0000000000400ee0 <phase_1>:
  400ee0:       48 83 ec 08             sub    $0x8,%rsp
  400ee4:       be 00 24 40 00          mov    $0x402400,%esi
  400ee9:       e8 4a 04 00 00          callq  401338 <strings_not_equal>
  400eee:       85 c0                   test   %eax,%eax
  400ef0:       74 05                   je     400ef7 <phase_1+0x17>
  400ef2:       e8 43 05 00 00          callq  40143a <explode_bomb>
  400ef7:       48 83 c4 08             add    $0x8,%rsp
  400efb:       c3                      retq   
```

- phase_1调用了`strings_not_equal`来判断输入的字符串是否和0x402400处字符串相等，相等则跳过炸弹引爆（过关）

```txt
root@0435fe0fdd6e:/ctf/work/TianWen/csapp/bomblab/bomb# objdump -s --start-address=0x402400 --stop-address=0x402480 ./bomb

./bomb:     file format elf64-x86-64

Contents of section .rodata:
 402400 426f7264 65722072 656c6174 696f6e73  Border relations
 402410 20776974 68204361 6e616461 20686176   with Canada hav
 402420 65206e65 76657220 6265656e 20626574  e never been bet
 402430 7465722e 00000000 576f7721 20596f75  ter.....Wow! You
 402440 27766520 64656675 73656420 74686520  've defused the 
 402450 73656372 65742073 74616765 2100666c  secret stage!.fl
 402460 79657273 00000000 00000000 00000000  yers............
 402470 7c0f4000 00000000 b90f4000 00000000  |.@.......@.....
```

- key:`Border relations with Canada have never been better.`

### phase_2

- 查看代码：

```txt
0000000000400efc <phase_2>:
  400efc:       55                      push   %rbp
  400efd:       53                      push   %rbx
  400efe:       48 83 ec 28             sub    $0x28,%rsp
  400f02:       48 89 e6                mov    %rsp,%rsi
  400f05:       e8 52 05 00 00          callq  40145c <read_six_numbers>
  400f0a:       83 3c 24 01             cmpl   $0x1,(%rsp)
  400f0e:       74 20                   je     400f30 <phase_2+0x34>
  400f10:       e8 25 05 00 00          callq  40143a <explode_bomb>
  400f15:       eb 19                   jmp    400f30 <phase_2+0x34>
  400f17:       8b 43 fc                mov    -0x4(%rbx),%eax
  400f1a:       01 c0                   add    %eax,%eax
  400f1c:       39 03                   cmp    %eax,(%rbx)
  400f1e:       74 05                   je     400f25 <phase_2+0x29>
  400f20:       e8 15 05 00 00          callq  40143a <explode_bomb>
  400f25:       48 83 c3 04             add    $0x4,%rbx
  400f29:       48 39 eb                cmp    %rbp,%rbx
  400f2c:       75 e9                   jne    400f17 <phase_2+0x1b>
  400f2e:       eb 0c                   jmp    400f3c <phase_2+0x40>
  400f30:       48 8d 5c 24 04          lea    0x4(%rsp),%rbx
  400f35:       48 8d 6c 24 18          lea    0x18(%rsp),%rbp
  400f3a:       eb db                   jmp    400f17 <phase_2+0x1b>
  400f3c:       48 83 c4 28             add    $0x28,%rsp
  400f40:       5b                      pop    %rbx
  400f41:       5d                      pop    %rbp
  400f42:       c3                      retq   
```

- phase_2从栈上（也就是用户输入的数据）读取六个数字输入，先和1比较，rbx作为下一个数字的指针，返回到400f17处比较rbx和eax*2，这里的eax是rbx的前一个数字，到400f29处判断是否全部比较完毕，形成一个循环的比较，即后一个数永远是前一个数的2倍

- key: `1 2 4 8 16 32`

### phase_3

```txt
0000000000400f43 <phase_3>:
  400f43:       48 83 ec 18             sub    $0x18,%rsp
  400f47:       48 8d 4c 24 0c          lea    0xc(%rsp),%rcx
  400f4c:       48 8d 54 24 08          lea    0x8(%rsp),%rdx
  400f51:       be cf 25 40 00          mov    $0x4025cf,%esi               ; "%d %d" 接受两个整数输入
  400f56:       b8 00 00 00 00          mov    $0x0,%eax
  400f5b:       e8 90 fc ff ff          callq  400bf0 <__isoc99_sscanf@plt>
  400f60:       83 f8 01                cmp    $0x1,%eax
  400f63:       7f 05                   jg     400f6a <phase_3+0x27>
  400f65:       e8 d0 04 00 00          callq  40143a <explode_bomb>
  400f6a:       83 7c 24 08 07          cmpl   $0x7,0x8(%rsp)
  400f6f:       77 3c                   ja     400fad <phase_3+0x6a>        ; 大于7则引爆
  400f71:       8b 44 24 08             mov    0x8(%rsp),%eax
  400f75:       ff 24 c5 70 24 40 00    jmpq   *0x402470(,%rax,8)           ; 即*(0x402070 + rax*8)指向400f7c，402070为switch的跳转表首地址
  400f7c:       b8 cf 00 00 00          mov    $0xcf,%eax
  400f81:       eb 3b                   jmp    400fbe <phase_3+0x7b>
  400f83:       b8 c3 02 00 00          mov    $0x2c3,%eax
  400f88:       eb 34                   jmp    400fbe <phase_3+0x7b>
  400f8a:       b8 00 01 00 00          mov    $0x100,%eax
  400f8f:       eb 2d                   jmp    400fbe <phase_3+0x7b>
  400f91:       b8 85 01 00 00          mov    $0x185,%eax
  400f96:       eb 26                   jmp    400fbe <phase_3+0x7b>
  400f98:       b8 ce 00 00 00          mov    $0xce,%eax
  400f9d:       eb 1f                   jmp    400fbe <phase_3+0x7b>
  400f9f:       b8 aa 02 00 00          mov    $0x2aa,%eax
  400fa4:       eb 18                   jmp    400fbe <phase_3+0x7b>
  400fa6:       b8 47 01 00 00          mov    $0x147,%eax
  400fab:       eb 11                   jmp    400fbe <phase_3+0x7b>
  400fad:       e8 88 04 00 00          callq  40143a <explode_bomb>
  400fb2:       b8 00 00 00 00          mov    $0x0,%eax
  400fb7:       eb 05                   jmp    400fbe <phase_3+0x7b>
  400fb9:       b8 37 01 00 00          mov    $0x137,%eax                  ; 当rax=1（switch-case 1）时，判断第二个输入数字为311
  400fbe:       3b 44 24 0c             cmp    0xc(%rsp),%eax
  400fc2:       74 05                   je     400fc9 <phase_3+0x86>
  400fc4:       e8 71 04 00 00          callq  40143a <explode_bomb>
  400fc9:       48 83 c4 18             add    $0x18,%rsp
  400fcd:       c3                      retq   
```

- key: `1 311`（其他的几个case只要找对了数字的组合应该也都可以）

### phase_4

```txt
000000000040100c <phase_4>:
  40100c:       48 83 ec 18             sub    $0x18,%rsp
  401010:       48 8d 4c 24 0c          lea    0xc(%rsp),%rcx           ; num 2 -- [rsp+0xc]
  401015:       48 8d 54 24 08          lea    0x8(%rsp),%rdx           ; num 1 -- [rsp+0x8]
  40101a:       be cf 25 40 00          mov    $0x4025cf,%esi           ; "%d %d"
  40101f:       b8 00 00 00 00          mov    $0x0,%eax
  401024:       e8 c7 fb ff ff          callq  400bf0 <__isoc99_sscanf@plt>
  401029:       83 f8 02                cmp    $0x2,%eax
  40102c:       75 07                   jne    401035 <phase_4+0x29>
  40102e:       83 7c 24 08 0e          cmpl   $0xe,0x8(%rsp)           ; num1 <= 0xe
  401033:       76 05                   jbe    40103a <phase_4+0x2e>
  401035:       e8 00 04 00 00          callq  40143a <explode_bomb>
  40103a:       ba 0e 00 00 00          mov    $0xe,%edx                ; arg3: 0xe
  40103f:       be 00 00 00 00          mov    $0x0,%esi                ; arg2: 0
  401044:       8b 7c 24 08             mov    0x8(%rsp),%edi           ; arg1: num1
  401048:       e8 81 ff ff ff          callq  400fce <func4>
  40104d:       85 c0                   test   %eax,%eax          
  40104f:       75 07                   jne    401058 <phase_4+0x4c>    ; 过关需要eax == 0
  401051:       83 7c 24 0c 00          cmpl   $0x0,0xc(%rsp)           ; num2 = 0
  401056:       74 05                   je     40105d <phase_4+0x51>
  401058:       e8 dd 03 00 00          callq  40143a <explode_bomb>
  40105d:       48 83 c4 18             add    $0x18,%rsp
  401061:       c3                      retq

0000000000400fce <func4>:
  400fce:       48 83 ec 08             sub    $0x8,%rsp
  400fd2:       89 d0                   mov    %edx,%eax                ; eax = edx(arg3)
  400fd4:       29 f0                   sub    %esi,%eax                ; eax -= esi(arg2)
  400fd6:       89 c1                   mov    %eax,%ecx                ; ecx = eax
  400fd8:       c1 e9 1f                shr    $0x1f,%ecx               ; ecx右移31位（保留符号位）
  400fdb:       01 c8                   add    %ecx,%eax                ; eax += ecx
  400fdd:       d1 f8                   sar    %eax                     ; eax /= 2
  400fdf:       8d 0c 30                lea    (%rax,%rsi,1),%ecx       ; ecx = rax + rsi = (arg3-arg2)/2 + arg2 = (arg2+arg3)/2
  400fe2:       39 f9                   cmp    %edi,%ecx                ; ecx <= edi则跳转到f2处
  400fe4:       7e 0c                   jle    400ff2 <func4+0x24>
  400fe6:       8d 51 ff                lea    -0x1(%rcx),%edx          ; edx = rcx - 1
  400fe9:       e8 e0 ff ff ff          callq  400fce <func4>           
  400fee:       01 c0                   add    %eax,%eax
  400ff0:       eb 15                   jmp    401007 <func4+0x39>      ; 
  400ff2:       b8 00 00 00 00          mov    $0x0,%eax
  400ff7:       39 f9                   cmp    %edi,%ecx
  400ff9:       7d 0c                   jge    401007 <func4+0x39>      ; ecx >= edi 则结束递归
  400ffb:       8d 71 01                lea    0x1(%rcx),%esi           ; esi = rcx + 1
  400ffe:       e8 cb ff ff ff          callq  400fce <func4>
  401003:       8d 44 00 01             lea    0x1(%rax,%rax,1),%eax    ; eax = 2*eax+1（返回值）
  401007:       48 83 c4 08             add    $0x8,%rsp
  40100b:       c3                      retq  
```

- 函数开始读取了两个整数，之后调用了`fun4(num1, 0, 14)`，根据fun4汇编大致整理出fun4的C语言描述:

```c
int func4(int num, int start, int end){
    int n = (end + start)/2; // ecx
    if(n > num){
        return 2 * func4(num, start, n-1);
    }else if (n < num){
        return 2 * func4(num, start+1, n) + 1;
    }else{
        return 0;
    }
}
```

- 根据题意需要`func4()`返回0，所以只要永远不满足`n < num`即可，由题意，第一次调用fun4时，n的值为7，所以输入7即可保证func4返回0

- key: `7 0 DrEvil`
    - 这里的DrEvil用于开启隐藏关

### phase_5

```txt
0000000000401062 <phase_5>:
  401062:       53                      push   %rbx
  401063:       48 83 ec 20             sub    $0x20,%rsp
  401067:       48 89 fb                mov    %rdi,%rbx
  40106a:       64 48 8b 04 25 28 00    mov    %fs:0x28,%rax
  401071:       00 00 
  401073:       48 89 44 24 18          mov    %rax,0x18(%rsp)
  401078:       31 c0                   xor    %eax,%eax
  40107a:       e8 9c 02 00 00          callq  40131b <string_length>
  40107f:       83 f8 06                cmp    $0x6,%eax                ; 字符串长度为6
  401082:       74 4e                   je     4010d2 <phase_5+0x70>
  401084:       e8 b1 03 00 00          callq  40143a <explode_bomb>
  401089:       eb 47                   jmp    4010d2 <phase_5+0x70>
  40108b:       0f b6 0c 03             movzbl (%rbx,%rax,1),%ecx       ; ecx = rbx + rax
  40108f:       88 0c 24                mov    %cl,(%rsp)               ; [rsp] = cl
  401092:       48 8b 14 24             mov    (%rsp),%rdx              ; rdx = [rsp]
  401096:       83 e2 0f                and    $0xf,%edx                ; edx保留低4位
  401099:       0f b6 92 b0 24 40 00    movzbl 0x4024b0(%rdx),%edx      ; edx = (0x4024b0 + rdx)
                                                                        ; 即以edx为索引从"maduiersnfotvbyl"取出一个字节（零扩展到双字，用edx存储）
  4010a0:       88 54 04 10             mov    %dl,0x10(%rsp,%rax,1)    ; 取出的字节存储到[rsp+rax+0x10]
  4010a4:       48 83 c0 01             add    $0x1,%rax                ; rax ++
  4010a8:       48 83 f8 06             cmp    $0x6,%rax                ; rax ?= 6
  4010ac:       75 dd                   jne    40108b <phase_5+0x29>
  4010ae:       c6 44 24 16 00          movb   $0x0,0x16(%rsp)          ; 循环取出字节结束 
  4010b3:       be 5e 24 40 00          mov    $0x40245e,%esi           ; 结果应当为"flyers"
  4010b8:       48 8d 7c 24 10          lea    0x10(%rsp),%rdi
  4010bd:       e8 76 02 00 00          callq  401338 <strings_not_equal>
  4010c2:       85 c0                   test   %eax,%eax
  4010c4:       74 13                   je     4010d9 <phase_5+0x77>
  4010c6:       e8 6f 03 00 00          callq  40143a <explode_bomb>
  4010cb:       0f 1f 44 00 00          nopl   0x0(%rax,%rax,1)
  4010d0:       eb 07                   jmp    4010d9 <phase_5+0x77>
  4010d2:       b8 00 00 00 00          mov    $0x0,%eax
  4010d7:       eb b2                   jmp    40108b <phase_5+0x29>
  4010d9:       48 8b 44 24 18          mov    0x18(%rsp),%rax
  4010de:       64 48 33 04 25 28 00    xor    %fs:0x28,%rax
  4010e5:       00 00 
  4010e7:       74 05                   je     4010ee <phase_5+0x8c>
  4010e9:       e8 42 fa ff ff          callq  400b30 <__stack_chk_fail@plt>
  4010ee:       48 83 c4 20             add    $0x20,%rsp
  4010f2:       5b                      pop    %rbx
  4010f3:       c3                      retq   
```

- 阅读代码，分析逻辑为：
    1. 输入六个字节
    2. 字节取低4位作为索引
    3. 按索引查找字节作为结果字符串
    4. 结果字符串与"flyers"对比
- key: `IONEFG`（小写也可，因为a=0x61，A=0x41，模16结果一致）

### phase_6

```txt
00000000004010f4 <phase_6>:
  4010f4:       41 56                   push   %r14
  4010f6:       41 55                   push   %r13
  4010f8:       41 54                   push   %r12
  4010fa:       55                      push   %rbp
  4010fb:       53                      push   %rbx
  4010fc:       48 83 ec 50             sub    $0x50,%rsp
  401100:       49 89 e5                mov    %rsp,%r13
  401103:       48 89 e6                mov    %rsp,%rsi                ; 向[rsp]读入六个数
  401106:       e8 51 03 00 00          callq  40145c <read_six_numbers>
  40110b:       49 89 e6                mov    %rsp,%r14
  40110e:       41 bc 00 00 00 00       mov    $0x0,%r12d               ; r12d = 0
  401114:       4c 89 ed                mov    %r13,%rbp
  401117:       41 8b 45 00             mov    0x0(%r13),%eax           ; eax = r13[0]
  40111b:       83 e8 01                sub    $0x1,%eax                ; eax --
  40111e:       83 f8 05                cmp    $0x5,%eax                ; eax <= 5 ?
  401121:       76 05                   jbe    401128 <phase_6+0x34>    ; 检查输入数字的合法性（1-6）
  401123:       e8 12 03 00 00          callq  40143a <explode_bomb>
  401128:       41 83 c4 01             add    $0x1,%r12d               ; r12d ++
  40112c:       41 83 fc 06             cmp    $0x6,%r12d               ; r12d ?= 6
  401130:       74 21                   je     401153 <phase_6+0x5f>    ; 跳出循环（r12d为外层循环计数）
  401132:       44 89 e3                mov    %r12d,%ebx
  401135:       48 63 c3                movslq %ebx,%rax                ; 内层循环
  401138:       8b 04 84                mov    (%rsp,%rax,4),%eax       ; （从输入中）读取下一个数字
  40113b:       39 45 00                cmp    %eax,0x0(%rbp)
  40113e:       75 05                   jne    401145 <phase_6+0x51>    ; 输入数字互不相等
  401140:       e8 f5 02 00 00          callq  40143a <explode_bomb>
  401145:       83 c3 01                add    $0x1,%ebx
  401148:       83 fb 05                cmp    $0x5,%ebx
  40114b:       7e e8                   jle    401135 <phase_6+0x41>
  40114d:       49 83 c5 04             add    $0x4,%r13                ; 指向下一个输入数字
  401151:       eb c1                   jmp    401114 <phase_6+0x20>

  401153:       48 8d 74 24 18          lea    0x18(%rsp),%rsi
  401158:       4c 89 f0                mov    %r14,%rax
  40115b:       b9 07 00 00 00          mov    $0x7,%ecx

  401160:       89 ca                   mov    %ecx,%edx
  401162:       2b 10                   sub    (%rax),%edx
  401164:       89 10                   mov    %edx,(%rax)
  401166:       48 83 c0 04             add    $0x4,%rax
  40116a:       48 39 f0                cmp    %rsi,%rax
  40116d:       75 f1                   jne    401160 <phase_6+0x6c>        ; 循环，将输入用7减，进行变换

  40116f:       be 00 00 00 00          mov    $0x0,%esi
  401174:       eb 21                   jmp    401197 <phase_6+0xa3>
  401176:       48 8b 52 08             mov    0x8(%rdx),%rdx               ; 指向下一节点
  40117a:       83 c0 01                add    $0x1,%eax
  40117d:       39 c8                   cmp    %ecx,%eax
  40117f:       75 f5                   jne    401176 <phase_6+0x82>
  401181:       eb 05                   jmp    401188 <phase_6+0x94>
  401183:       ba d0 32 60 00          mov    $0x6032d0,%edx               ; 链表第一个节点
  401188:       48 89 54 74 20          mov    %rdx,0x20(%rsp,%rsi,2)       ; 把每一个节点的指针复制到栈上
  40118d:       48 83 c6 04             add    $0x4,%rsi
  401191:       48 83 fe 18             cmp    $0x18,%rsi
  401195:       74 14                   je     4011ab <phase_6+0xb7>
  401197:       8b 0c 34                mov    (%rsp,%rsi,1),%ecx           ; id = 1 开始遍历链表
  40119a:       83 f9 01                cmp    $0x1,%ecx
  40119d:       7e e4                   jle    401183 <phase_6+0x8f>
  40119f:       b8 01 00 00 00          mov    $0x1,%eax
  4011a4:       ba d0 32 60 00          mov    $0x6032d0,%edx               ; 
  4011a9:       eb cb                   jmp    401176 <phase_6+0x82>

  4011ab:       48 8b 5c 24 20          mov    0x20(%rsp),%rbx              ; 复制完毕
  4011b0:       48 8d 44 24 28          lea    0x28(%rsp),%rax
  4011b5:       48 8d 74 24 50          lea    0x50(%rsp),%rsi
  4011ba:       48 89 d9                mov    %rbx,%rcx
  4011bd:       48 8b 10                mov    (%rax),%rdx
  4011c0:       48 89 51 08             mov    %rdx,0x8(%rcx)
  4011c4:       48 83 c0 08             add    $0x8,%rax
  4011c8:       48 39 f0                cmp    %rsi,%rax
  4011cb:       74 05                   je     4011d2 <phase_6+0xde>
  4011cd:       48 89 d1                mov    %rdx,%rcx
  4011d0:       eb eb                   jmp    4011bd <phase_6+0xc9>
  4011d2:       48 c7 42 08 00 00 00    movq   $0x0,0x8(%rdx)
  4011d9:       00 
  4011da:       bd 05 00 00 00          mov    $0x5,%ebp
  4011df:       48 8b 43 08             mov    0x8(%rbx),%rax               ; 下一个节点的data字段
  4011e3:       8b 00                   mov    (%rax),%eax
  4011e5:       39 03                   cmp    %eax,(%rbx)                  ; 比较当前节点的data和下一节点的data
  4011e7:       7d 05                   jge    4011ee <phase_6+0xfa>        ; 当前data需要大于等于下一个data，即从大到小排序
                                                                            ; 由于使用的是用7减过的输入作为id进行排序，因此输入的序号实际上是从小到大排列
  4011e9:       e8 4c 02 00 00          callq  40143a <explode_bomb>
  4011ee:       48 8b 5b 08             mov    0x8(%rbx),%rbx
  4011f2:       83 ed 01                sub    $0x1,%ebp
  4011f5:       75 e8                   jne    4011df <phase_6+0xeb>
  4011f7:       48 83 c4 50             add    $0x50,%rsp
  4011fb:       5b                      pop    %rbx
  4011fc:       5d                      pop    %rbp
  4011fd:       41 5c                   pop    %r12
  4011ff:       41 5d                   pop    %r13
  401201:       41 5e                   pop    %r14
  401203:       c3                      retq

Contents of section .data:
 6032d0 4c010000 01000000 e0326000 00000000  L........2`.....
 6032e0 a8000000 02000000 f0326000 00000000  .........2`.....
 6032f0 9c030000 03000000 00336000 00000000  .........3`.....
 603300 b3020000 04000000 10336000 00000000  .........3`.....
 603310 dd010000 05000000 20336000 00000000  ........ 3`.....
 603320 bb010000 06000000 00000000 00000000  ................

```

- 观察6032d0处，发现一组有规律的数据，观察内存发现结构体内部存在指向相邻空间下一结构体的指针，进一步推测结构体构造为单链表

```c
struct node{
    int data;
    int id;
    struct node* next;
};
```

- 经过调试证实上述猜想，并进一步得出输入数字被用作node的id进行排序，且过关需要将节点按data值从小到大排列
- key: `4 3 2 1 6 5`

### secret_phase

```txt
0000000000401242 <secret_phase>:
  401242:       53                      push   %rbx
  401243:       e8 56 02 00 00          callq  40149e <read_line>
  401248:       ba 0a 00 00 00          mov    $0xa,%edx                        ; 转为十进制
  40124d:       be 00 00 00 00          mov    $0x0,%esi
  401252:       48 89 c7                mov    %rax,%rdi                        ; 转换对象为read_line的输入
  401255:       e8 76 f9 ff ff          callq  400bd0 <strtol@plt>              ; 将输入字符串按第三个参数（base）转为长整型
  40125a:       48 89 c3                mov    %rax,%rbx
  40125d:       8d 40 ff                lea    -0x1(%rax),%eax                  ; eax -- 
  401260:       3d e8 03 00 00          cmp    $0x3e8,%eax                      ; eax <= 1000 ?
  401265:       76 05                   jbe    40126c <secret_phase+0x2a>
  401267:       e8 ce 01 00 00          callq  40143a <explode_bomb>
  40126c:       89 de                   mov    %ebx,%esi
  40126e:       bf f0 30 60 00          mov    $0x6030f0,%edi                   ; 0x24
  401273:       e8 8c ff ff ff          callq  401204 <fun7>
  401278:       83 f8 02                cmp    $0x2,%eax                        ; eax == 2
  40127b:       74 05                   je     401282 <secret_phase+0x40>
  40127d:       e8 b8 01 00 00          callq  40143a <explode_bomb>
  401282:       bf 38 24 40 00          mov    $0x402438,%edi
  401287:       e8 84 f8 ff ff          callq  400b10 <puts@plt>
  40128c:       e8 33 03 00 00          callq  4015c4 <phase_defused>
  401291:       5b                      pop    %rbx
  401292:       c3      

0000000000401204 <fun7>:
  401204:       48 83 ec 08             sub    $0x8,%rsp
  401208:       48 85 ff                test   %rdi,%rdi
  40120b:       74 2b                   je     401238 <fun7+0x34>
  40120d:       8b 17                   mov    (%rdi),%edx
  40120f:       39 f2                   cmp    %esi,%edx
  401211:       7e 0d                   jle    401220 <fun7+0x1c>
  401213:       48 8b 7f 08             mov    0x8(%rdi),%rdi
  401217:       e8 e8 ff ff ff          callq  401204 <fun7>
  40121c:       01 c0                   add    %eax,%eax
  40121e:       eb 1d                   jmp    40123d <fun7+0x39>
  401220:       b8 00 00 00 00          mov    $0x0,%eax
  401225:       39 f2                   cmp    %esi,%edx
  401227:       74 14                   je     40123d <fun7+0x39>
  401229:       48 8b 7f 10             mov    0x10(%rdi),%rdi
  40122d:       e8 d2 ff ff ff          callq  401204 <fun7>
  401232:       8d 44 00 01             lea    0x1(%rax,%rax,1),%eax
  401236:       eb 05                   jmp    40123d <fun7+0x39>
  401238:       b8 ff ff ff ff          mov    $0xffffffff,%eax
  40123d:       48 83 c4 08             add    $0x8,%rsp
  401241:       c3                      retq   


Contents of section .data:
 6030f0 24000000 00000000 10316000 00000000  $........1`.....
 603100 30316000 00000000 00000000 00000000  01`.............
 603110 08000000 00000000 90316000 00000000  .........1`.....
 603120 50316000 00000000 00000000 00000000  P1`.............
 603130 32000000 00000000 70316000 00000000  2.......p1`.....
 603140 b0316000 00000000 00000000 00000000  .1`.............
 603150 16000000 00000000 70326000 00000000  ........p2`.....
 603160 30326000 00000000 00000000 00000000  02`.............
 603170 2d000000 00000000 d0316000 00000000  -........1`.....
 603180 90326000 00000000 00000000 00000000  .2`.............
 603190 06000000 00000000 f0316000 00000000  .........1`.....
 6031a0 50326000 00000000 00000000 00000000  P2`.............
 6031b0 6b000000 00000000 10326000 00000000  k........2`.....
 6031c0 b0326000 00000000 00000000 00000000  .2`.............
 6031d0 28000000 00000000 00000000 00000000  (...............
 6031e0 00000000 00000000 00000000 00000000  ................
 6031f0 01000000 00000000 00000000 00000000  ................
```

- fun7的分析过程和func4很相似，这里直接给出一个大致的C语言描述

```c
int fun7(int arg1, int arg2){
    if(!(arg1)){
        return 0xffffffff;
    }
    if(arg1 > arg2){
        return 2 * fun7(*(&arg1 + 8), arg2); // 左子树
    }
    int r = 0;
    else if(arg1 != arg2){
        return 2 * fun7(*(&arg1 + 0x10), arg2) + 1; // 右子树
    }
    return r;
}
```

- 结合`secret_phase`的代码，需要控制fun7返回值为2
- 观察数据，每个结构体存在一个数据和两个指针，推断出应为二叉树，根据数据还原，得出一棵完全二叉树
- 根据树反推返回值即可

- key: `20`

### 总结

- 直接看objdump的反汇编很累，用了r2看跳转，实在不想手绘cfg
- 还要多提升自己的阅读理解速度
- 最终答案文件保存在`key`文件中

执行结果
```txt
root@0435fe0fdd6e:/ctf/work/TianWen/csapp/bomblab/bomb# ./bomb < key
Welcome to my fiendish little bomb. You have 6 phases with
which to blow yourself up. Have a nice day!
Phase 1 defused. How about the next one?
That's number 2.  Keep going!
Halfway there!
So you got that one.  Try this one.
Good work!  On to the next...
Curses, you've found the secret phase!
But finding it and solving it are quite different...
Wow! You've defused the secret stage!
Congratulations! You've defused the bomb!
```

## attacklab

没难度，基础pwn题，不写笔记了（逃

- 最后一个phase有点绕，抠了半天搞出来的ROP居然有bug，后来发现是找错了ROP（把获取rsp的gadget找成了获取esp的），裂开

## archlab

- part A就是简单的汇编程序
- part B就是按fetch、decode、execute、memory、write back、这几个阶段实现一个iaddq，很简单，框架都是现成的。
- part C很难，需要解决流水线的数据冒险，同时考虑到各种优化问题，需要把第四章后半部分和第五章都搞定，这个等其他lab做完了再回来研究。

## cachelab

- 待做

## shlab

- 仔细看好实验要求，把书过一遍，就能很快完成编码。
- 但还是遇到了一个bug，找了一个晚上，发现是eval中`sigprocmask(SIG_UNBLOCK, &set, NULL);`的`SIG_UNBLOCK`被用成了`SIG_SETMASK`，因为之前有bug的时候参考了一些文章，当时以为其他bug是由于UNBLOCK引起的，所以事先把这里修改了。从名字上看不应该犯这个错误。尤其是找了一个大晚上，很伤。
  - bug会表现为在fg执行的命令会一直阻塞下去
- 应该还有点小bug，比如输出内容和参考内容有些不一致这些，没能一遍写出来还是代码能力太差，这个实验一共也就写了200行左右。剩下的不准备搞了，毕竟实验目的是学习异常控制流的东西，再纠结细节下去太浪费时间，时间很宝贵。

## malloc

## proxy
