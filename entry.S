# The xv6 kernel starts executing in this file. This file is linked with
# the kernel C code, so it can refer to kernel symbols such as main().
# The boot block (bootasm.S and bootmain.c) jumps to entry below.
# xv6内核的入口，这个文件跟 kernel.c 相链接，因此可以使用诸如 main() 之类的内核符号
        
# Multiboot header, for multiboot boot loaders like GNU Grub.
# http://www.gnu.org/software/grub/manual/multiboot/multiboot.html
#
# Using GRUB 2, you can boot xv6 from a file stored in a
# Linux file system by copying kernel or kernelmemfs to /boot
# and then adding this menu entry:
#
# menuentry "xv6" {
# 	insmod ext2
# 	set root='(hd0,msdos1)'
# 	set kernel='/boot/kernel'
# 	echo "Loading ${kernel}..."
# 	multiboot ${kernel} ${kernel}
# 	boot
# }

#include "asm.h"
#include "memlayout.h"
#include "mmu.h"
#include "param.h"

# Multiboot header.  Data to direct multiboot loader.
#.p2align 是用于指定程序的对齐方式，这类似于结构体的字节对齐，为的是加速程序的执行速度，p2align 的单位是指数，即按照 2 的 exp 次方对齐，上文中的 .p2align 2 即为按照 2^2 = 4 字节对齐，也就是说，如果单行指令或数据的长度不足4字节，将用 0 补全，超过 4 但不是 4 的倍数，则按照最小倍数补全；
.p2align 2 
.text
#.globl 说明 变量符号 counter 是一个全局变量，即可在其他文件中通过 extern 的方式引入；
# 多重引导文件头
#   0 u32 magic     require
#   1 u32 flags     require
#   2 u32 checksum  require
.globl multiboot_header
multiboot_header:
  #define magic 0x1badb002
  #define flags 0
  .long magic
  .long flags
  .long (-magic-flags)

# By convention, the _start symbol specifies the ELF entry point.
# Since we haven't set up virtual memory yet, our entry point is
# the physical address of 'entry'.
# kernel的入口仍然是低地址值，因为此时分页还没有开启
.globl _start
_start = V2P_WO(entry)

# Entering xv6 on boot processor, with paging off.
# 开启分页
.globl entry
entry:
  # Turn on page size extension for 4Mbyte pages
  # cr4寄存器的PSE位置为1，那么CPU自动使用4M大小的内存页, 设置cr4，使用4M页
  movl    %cr4, %eax
  orl     $(CR4_PSE), %eax
  movl    %eax, %cr4
  # Set page directory
  # 设置cr3, 目录entrypgdir 看 main.c
  movl    $(V2P_WO(entrypgdir)), %eax
  movl    %eax, %cr3
  # Turn on paging.
  # 启动分页
  movl    %cr0, %eax
  orl     $(CR0_PG|CR0_WP), %eax
  movl    %eax, %cr0

  # Set up the stack pointer.
  # 创建CPU栈，这个是该CPU独有的，启动其他CPU时，每个都自己的stack
  movl $(stack + KSTACKSIZE), %esp

  # Jump to main(), and switch to executing at
  # high addresses. The indirect call is needed because
  # the assembler produces a PC-relative instruction
  # for a direct jump.
  # 进入高地址空间(2GB以上)
  mov $main, %eax
  jmp *%eax
# common symbol，开辟stack区域，大小为KSTACKSIZE
# .common: 声明未初始化的数据的通用内存区域
.comm stack, KSTACKSIZE
