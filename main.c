#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "x86.h"

static void startothers(void);
static void mpmain(void)  __attribute__((noreturn));
extern pde_t *kpgdir;
extern char end[]; // first address after kernel loaded from ELF file

// Bootstrap processor starts running C code here.
// Allocate a real stack and switch to it, first
// doing some setup required for memory allocator to work.
int
main(void)
{
  // 初始化kernel img结束位置直到4M之间的虚拟内存，kernel code 从1MB处开始，大约有1MB的大小，所以剩余2MB可以用来kalloc
  kinit1(end, P2V(4*1024*1024)); // phys page allocator
  kvmalloc();      // kernel page table， 分配一页内存，然后把内存页切换到这一页上
  mpinit();        // detect other processors 检测处理器,并获取对应的apicid，cpu数量
  lapicinit();     // interrupt controller  初始化Local Advanced Programmable Interrupt Controller
  seginit();       // segment descriptors 初始化段寄存器，注意，logical address指的是”段内地址”，virtual address又叫做线性地址，这里的设置把所有段(除gs段)基址均设为0，这样实际就取消分段机制了。
  picinit();       // disable pic 关闭pic中断处理器
  ioapicinit();    // another interrupt controller 初始化ioapic， 处理外部io设备产生的中断
  consoleinit();   // console hardware 初始化console设备
  uartinit();      // serial port, Universal Asynchronous Receiver
  pinit();         // process table 初始化进程表锁
  tvinit();        // trap vectors 初始化中断向量表，包括系统中断和硬件中断等
  binit();         // buffer cache 初始化缓冲区
  fileinit();      // file table 初始化文件表锁
  ideinit();       // disk ide硬盘初始化
  startothers();   // start other processors 启动其他处理器
  // 初始化4M直到PHYSTOP部分的内存
  kinit2(P2V(4*1024*1024), P2V(PHYSTOP)); // must come after startothers()
  userinit();      // first user process 初始化第一个用户进程
  mpmain();        // finish this processor's setup
}

// Other CPUs jump here from entryother.S.
static void
mpenter(void)
{
  switchkvm();
  seginit();
  lapicinit();
  mpmain();
}

// Common CPU setup code.
static void
mpmain(void)
{
  cprintf("cpu%d: starting %d\n", cpuid(), cpuid());
  idtinit();       // load idt register
  xchg(&(mycpu()->started), 1); // tell startothers() we're up
  scheduler();     // start running processes
}

pde_t entrypgdir[];  // For entry.S

// Start the non-boot (AP) processors.
static void
startothers(void)
{
  extern uchar _binary_entryother_start[], _binary_entryother_size[];
  uchar *code;
  struct cpu *c;
  char *stack;

  // Write entry code to unused memory at 0x7000.
  // The linker has placed the image of entryother.S in
  // _binary_entryother_start.
  code = P2V(0x7000);
  memmove(code, _binary_entryother_start, (uint)_binary_entryother_size);

  for(c = cpus; c < cpus+ncpu; c++){
    if(c == mycpu())  // We've started already.
      continue;

    // Tell entryother.S what stack to use, where to enter, and what
    // pgdir to use. We cannot use kpgdir yet, because the AP processor
    // is running in low  memory, so we use entrypgdir for the APs too.
    stack = kalloc();
    *(void**)(code-4) = stack + KSTACKSIZE;
    *(void(**)(void))(code-8) = mpenter;
    *(int**)(code-12) = (void *) V2P(entrypgdir);

    lapicstartap(c->apicid, V2P(code));

    // wait for cpu to finish mpmain()
    while(c->started == 0)
      ;
  }
}

// The boot page table used in entry.S and entryother.S.
// Page directories (and page tables) must start on page boundaries,
// hence the __aligned__ attribute.
// PTE_PS in a page directory entry enables 4Mbyte pages.
// 每个 PTE 都包含一些标志位，说明分页硬件对应的虚拟地址的使用权限。
// PTE_P 表示 PTE 是否陈列在页表中：如果不是，那么一个对该页的引用会引发错误（也就是：不允许被使用）。
// PTE_W 控制着能否对页执行写操作；如果不能，则只允许对其进行读操作和取指令。
// PTE_U 控制着用户程序能否使用该页；如果不能，则只有内核能够使用该页。
__attribute__((__aligned__(PGSIZE)))
pde_t entrypgdir[NPDENTRIES] = {
  // Map VA's [0, 4MB) to PA's [0, 4MB)
  [0] = (0) | PTE_P | PTE_W | PTE_PS,
  // Map VA's [KERNBASE, KERNBASE+4MB) to PA's [0, 4MB)
  [KERNBASE>>PDXSHIFT] = (0) | PTE_P | PTE_W | PTE_PS,
};

//PAGEBREAK!
// Blank page.
//PAGEBREAK!
// Blank page.
//PAGEBREAK!
// Blank page.

