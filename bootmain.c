// Boot loader.
//
// Part of the boot block, along with bootasm.S, which calls bootmain().
// bootasm.S has put the processor into protected 32-bit mode.
// bootmain() loads an ELF kernel image from the disk starting at
// sector 1 and then jumps to the kernel entry routine.
// bootasm.S 将系统推进到32为保护模式了，bootmain将从磁盘加载kernel

#include "types.h"
#include "elf.h"
#include "x86.h"
#include "memlayout.h"

#define SECTSIZE  512

void readseg(uchar*, uint, uint);

void
bootmain(void)
{
  struct elfhdr *elf;
  struct proghdr *ph, *eph;
  void (*entry)(void);
  uchar* pa;

  elf = (struct elfhdr*)0x10000; // scratch space

  // Read 1st page off disk
  readseg((uchar*)elf, 4096, 0); // 读取kernel数据到内存0x10000 4KB处

  // Is this an ELF executable?，有效性判断
  if(elf->magic != ELF_MAGIC)
    return;  // let bootasm.S handle error

  // Load each program segment (ignores ph flags).
  // 除了第一个elf文件头，读取剩下的文件头(因为第一个已经读取了)
  ph = (struct proghdr*)((uchar*)elf + elf->phoff);
  eph = ph + elf->phnum;
  for(; ph < eph; ph++){
    pa = (uchar*)ph->paddr;
    readseg(pa, ph->filesz, ph->off);
    if(ph->memsz > ph->filesz)
      stosb(pa + ph->filesz, 0, ph->memsz - ph->filesz); // 从 pa + ph->filesz 位置开始， 将连续 ph->memsz - ph->filesz 个字节写成 0
  }

  // Call the entry point from the ELF header.
  // Does not return!
  entry = (void(*)(void))(elf->entry);
  entry();
}

void
waitdisk(void)
{
  // Wait for disk ready.
  // 检查磁盘是否有空
  while((inb(0x1F7) & 0xC0) != 0x40)
    ;
}

// Read a single sector at offset into dst.
// 通过io端口操作 操作硬盘控制器 读取硬盘
void
readsect(void *dst, uint offset)
{
  // Issue command.
  waitdisk();
  outb(0x1F2, 1);   // 用来放入要读写的扇区数量

  outb(0x1F3, offset); // 用来放入要读写的扇区号码
  outb(0x1F4, offset >> 8); // 用来存放读写柱面的低8位字节
  outb(0x1F5, offset >> 16); // 用来存放读写柱面的高2位字节(其高6位恒为0)
  outb(0x1F6, (offset >> 24) | 0xE0); // 用来存放要读/写的磁盘号及磁头号
  // 指令20读磁盘
  outb(0x1F7, 0x20);  // cmd 0x20 - read sectors

  // Read data.
  waitdisk();
  // 0x1f0是硬盘接口的数据端口，一旦硬盘控制器空闲，且准备就绪，就可以连续的从这个端口写入或读取数据
  insl(0x1F0, dst, SECTSIZE/4);// 把0x1F0的数据读入内存dst位置，循环SECTISIZE / 4次
}

// Read 'count' bytes at 'offset' from kernel into physical address 'pa'.
// Might copy more than asked.
// 从硬盘offset处读取count大小的数据存入内存pa处
void
readseg(uchar* pa, uint count, uint offset)
{
  uchar* epa;

  epa = pa + count;

  pa -= offset % SECTSIZE;

  // Translate from bytes to sectors; kernel starts at sector 1.
  offset = (offset / SECTSIZE) + 1;
  // offset = 1

  // If this is too slow, we could read lots of sectors at a time.
  // We'd write more to memory than asked, but it doesn't matter --
  // we load in increasing order.
  for(; pa < epa; pa += SECTSIZE, offset++)
    readsect(pa, offset);
}
