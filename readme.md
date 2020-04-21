## 阅读参考
1. [makefile分析](https://iluvrachel.github.io/2019/02/23/xv6-makefile-%E8%A7%A3%E6%9E%90/)
2. [中文文档](https://th0ar.gitbooks.io/xv6-chinese/content/content/AppendixB.html)
3. [入口程序+引导程序](http://ybin.cc/os/xv6-boot/)
4. [bootman详细注释](https://my.oschina.net/wangconglin87/blog/278745)
5. [mov语法](https://blog.csdn.net/m0_37806112/article/details/80549927)
6. [LGDT/LIDT语法](https://blog.csdn.net/judyge/article/details/52343632)
7. [ESP寄存器](https://blog.csdn.net/u011822516/article/**details**/20001765)
8. [ELF文件格式](https://blog.csdn.net/xuehuafeiwu123/article/details/72963229)
9. [内联汇编 asm volatile](https://blog.csdn.net/jmh1996/article/details/82860227)
   1. [更详细的解释]https://dillonzq.com/2019/08/c-%E8%AF%AD%E8%A8%80%E5%86%85%E8%81%94%E6%B1%87%E7%BC%96/#1-%E7%AE%80%E4%BB%8B
10. [CLD汇编指令](https://www.cnblogs.com/DeeLMind/p/6882949.html)
11. [多重引导文件头](https://www.gnu.org/software/grub/manual/multiboot/multiboot.html#Architecture)
12. [cr0-4寄存器](https://blog.csdn.net/epluguo/article/details/9260429)
13. [eip寄存器](https://blog.csdn.net/xzhang76/article/details/8158749)

14. [lapic](https://www.binss.me/blog/what-is-apic/)
15. [服务器体系(SMP, NUMA, MPP)与共享存储器架构(UMA和NUMA)](https://blog.csdn.net/gatieme/article/details/52098615)

## 启动系统
1. 从20位实模式进入32位保护模式（分段寻址）--- bootasm.S
   1. 通过io端口0x64和0x60控制系统进入32位模式
   2. 设置cr0第一位来进入保护模式
   3. 使用可以访问全部内存的code段和data段来做当前代码运行的临时段
   4. 设置esp寄存器为start，
2. 从硬盘装载真正的操作系统进入内存 --- bootasm.c
   1. 从硬盘第二个扇区读出elf文件头到内存4KB处
   2. 解析elf文件头包含的程序头，并读取程序进内存的1MB处
   3. 执行第一个程序 entry.S
3. 开启分页并新建两个4M页面 --- entry.S
4. 运行main.c初始化内存，进程等等
   1. kinit1 初始化end到4M之间的内存空间
   2. kvmalloc 分配一个4k内存页，并切换到对应的页上


## 寻址方式
1. 逻辑地址（段选择器：偏移）
2. 线性地址（段选择器：偏移 => 段选择器<<4 + 偏移 || 段选择器对应段的基址 + 偏移)
3. 物理地址（分页组件 ? 分页基址 + 线性地址 : 线性地址 ）