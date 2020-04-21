#include "asm.h"
#include "memlayout.h"
#include "mmu.h"

# Start the first CPU: switch to 32-bit protected mode, jump into C.
# The BIOS loads this code from the first sector of the hard disk into
# memory at physical address 0x7c00 and starts executing in real mode
# with %cs=0 %ip=7c00.
# BIOS从第一个扇区读进本程序，开始启动，当前物理地址是0x7c00, 硬盘寻址是实模式(通过 段寄存器<<2 来凑成20位物理地址)

.code16                       # Assemble for 16-bit mode， 使用16位汇编
.globl start
start:
  cli                         # BIOS enabled interrupts; disable， 关闭中断，防止启动的时候由于中断的产生而导致异常

  # Zero data segment registers DS, ES, and SS
  # 给四个寄存器初始化为0，实模式下当程序，用到一个内存地址时，
  # 处理器会自动在该地址上加上某个16位段寄存器值的16倍。
  # 因此，内存引用中其实隐含地使用了段寄存器的值：取指会用到 %cs ，读写数据会用到%ds ，读写栈会用到 %ss 。
  xorw    %ax,%ax             # Set %ax to zero
  movw    %ax,%ds             # -> Data Segment
  movw    %ax,%es             # -> Extra Segment
  movw    %ax,%ss             # -> Stack Segment

  # Physical address line A20 is tied to zero so that the first PCs 
  # with 2 MB would run software that assumed 1 MB.  Undo that.
  # 曾经IBM通过硬件来限制内存地址是20位，但是当他研发出超过20位的内存的时候，
  # 为了兼容，就设了一个规则，只要用引导加载器通过 I/O 指令控制端口 0x64 和 0x60 上的键盘控制器，
  # 使其输出端口的第2位为高位，超过20位的内存地址可以正常使用。
seta20.1:
  inb     $0x64,%al               # Wait for not busy
  testb   $0x2,%al
  jnz     seta20.1

  movb    $0xd1,%al               # 0xd1 -> port 0x64
  outb    %al,$0x64

seta20.2:
  inb     $0x64,%al               # Wait for not busy
  testb   $0x2,%al
  jnz     seta20.2

  movb    $0xdf,%al               # 0xdf -> port 0x60
  outb    %al,$0x60

  # Switch from real to protected mode.  Use a bootstrap GDT that makes
  # virtual addresses map directly to physical addresses so that the
  # effective memory map doesn't change during the transition.
  # 切换成保护模式来运行，使用bootstrap的GDT（段选择符表）
  # 将cr0设置成的第一位设置为1来切换成保护模式
  # CR0_PE === 0x00000001
  lgdt    gdtdesc
  movl    %cr0, %eax
  orl     $CR0_PE, %eax
  movl    %eax, %cr0

//PAGEBREAK!
  # Complete the transition to 32-bit protected mode by using a long jmp
  # to reload %cs and %eip.  The segment descriptors are set up with no
  # translation, so that the mapping is still the identity mapping.
  # 允许保护模式并不会马上改变处理器把逻辑地址翻译成物理地址的过程；只有当某个段寄存器加载了一个新的值，然后处理器通过这个值读取 GDT 的一项从而改变了内部的段设置。我们没法直接修改 %cs，所以使用了一个 ljmp 指令（8453）。
  # ljmp segment offset 设置下一条执行指令的段寄存器和段内偏移地址(EIP寄存器)
  # 由于第一个GDT是空的，所以要应该选第二个code段，+8字节的偏移，因此是 0x01<<3
  # SEG_KCODE === 1
  ljmp    $(SEG_KCODE<<3), $start32

.code32  # Tell assembler to generate 32-bit code now. 此处要生成32位的机器码了
start32:
  # Set up the protected-mode data segment registers
  # 进入保护模式后，要把各个段寄存器设置成（data段）也就是0x10<<3
  movw    $(SEG_KDATA<<3), %ax    # Our data segment selector
  movw    %ax, %ds                # -> DS: Data Segment
  movw    %ax, %es                # -> ES: Extra Segment
  movw    %ax, %ss                # -> SS: Stack Segment
  movw    $0, %ax                 # Zero segments not ready for use
  movw    %ax, %fs                # -> FS
  movw    %ax, %gs                # -> GS

  # Set up the stack pointer and call into C.
  # 栈指针寄存器(extended stack pointer)，其内存放着一个指针，该指针永远指向系统栈最上面一个栈帧的栈顶。 $start 0x7c00，栈是向低内存生长的，所以设置成0x7c00，代码和栈就不会互相干扰
  # 跳转执行bootmain.c
  movl    $start, %esp
  call    bootmain

  # If bootmain returns (it shouldn't), trigger a Bochs
  # breakpoint if running under Bochs, then loop.
  # 返回就是报错了
  movw    $0x8a00, %ax            # 0x8a00 -> port 0x8a00
  movw    %ax, %dx
  outw    %ax, %dx
  movw    $0x8ae0, %ax            # 0x8ae0 -> port 0x8a00
  outw    %ax, %dx
spin:
  jmp     spin

# Bootstrap GDT 临时的GDT段选择符表，GDT的第一个表项必须为空，然后xv6为bootstrap分配了code和data
# GDT一个表项用8个字节
# STA_W 0b00000010 可写， STA_X 0b00001000可执行, STA_R 0b00000010可读
# code段和data段在这里设置的是一样的
.p2align 2                                # force 4 byte alaignment
gdt:
  SEG_NULLASM                             # null seg
  SEG_ASM(STA_X|STA_R, 0x0, 0xffffffff)   # code seg 一个可写的code段
  SEG_ASM(STA_W, 0x0, 0xffffffff)         # data seg 一个可读的data段

gdtdesc:
  .word   (gdtdesc - gdt - 1)             # sizeof(gdt) - 1
  .long   gdt                             # address gdt

