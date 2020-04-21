// Routines to let C code use special x86 instructions.

/**
 * 内联汇编规则
 * asm [volatile] (AssemblerTemplate
 *    : [OutputOperands] // 格式如 [asmSymbolicName1] constraint (cvariablename1), [asmSymbolicName1] constraint (cvariablename1)···
 *    : [Inputoperands]
 *    : [Clobbers] // 修饰寄存器列表，表示那些寄存器正在以不可预测的方式读写， memory表示不可以缓存保存与寄存器的内存值
 * )
 * 
 * volatile 表示，编译器不要对这一段汇编进行优化，原样输出就好
 * AssemblerTemplate 是汇编模版，他是一个包含汇编指令的模版，可以通过使用占位符来替换
 * asmSymbolicName 是asm符号别名，而cvariablename是c语言变量别名 汇编通过 %[别名] 来访问这些变量，另外每一个数都会有编号， 通过 %[数字]访问
 * constraint 是寄存器操作数约束
 *    a %eax %ax %al
 *    b %ebx %eb %el
 *    c %ecx %cx %cl
 *    d	%edx %dx %dl
 *    S	%esi %si
 *    D	%edi %di
*/

static inline uchar
inb(ushort port)
{
  uchar data;

  asm volatile("in %1,%0" : "=a" (data) : "d" (port));
  return data;
}

/**
 * 把DF寄存器清零。
 * 把循环次数写进cx计数寄存器中。
 * 循环执行insl指令，把IO接口0x1F0的数据读取并写到对应的内存区域上，每次读取4个字节。
 * 每次循环会让cx计数寄存器的值减1，并更新DI寄存器的值，让它加4。
 * 这样，在循环结束时候，刚好读取完1个扇区的所有数据。
 * /
static inline void
insl(int port, void *addr, int cnt)
{
  asm volatile("cld; rep insl" :
               "=D" (addr), "=c" (cnt) :
               "d" (port), "0" (addr), "1" (cnt) :
               "memory", "cc");
}

static inline void
outb(ushort port, uchar data)
{
  asm volatile("out %0,%1" : : "a" (data), "d" (port));
}

static inline void
outw(ushort port, ushort data)
{
  asm volatile("out %0,%1" : : "a" (data), "d" (port));
}

static inline void
outsl(int port, const void *addr, int cnt)
{
  asm volatile("cld; rep outsl" :
               "=S" (addr), "=c" (cnt) :
               "d" (port), "0" (addr), "1" (cnt) :
               "cc");
}

static inline void
stosb(void *addr, int data, int cnt)
{
  asm volatile("cld; rep stosb" : // rep stosb就是从EDI所指的内存开始，将连续的ECX个字节写成AL的内容
               "=D" (addr), "=c" (cnt) : // D	%edi  c %ecx 
               "0" (addr), "1" (cnt), "a" (data) : // a %eax
               "memory", "cc");
}

static inline void
stosl(void *addr, int data, int cnt)
{
  asm volatile("cld; rep stosl" :
               "=D" (addr), "=c" (cnt) :
               "0" (addr), "1" (cnt), "a" (data) :
               "memory", "cc");
}

struct segdesc;

static inline void
lgdt(struct segdesc *p, int size)
{
  volatile ushort pd[3];

  pd[0] = size-1;
  pd[1] = (uint)p;
  pd[2] = (uint)p >> 16;

  asm volatile("lgdt (%0)" : : "r" (pd));
}

struct gatedesc;

static inline void
lidt(struct gatedesc *p, int size)
{
  volatile ushort pd[3];

  pd[0] = size-1;
  pd[1] = (uint)p;
  pd[2] = (uint)p >> 16;

  asm volatile("lidt (%0)" : : "r" (pd));
}

static inline void
ltr(ushort sel)
{
  asm volatile("ltr %0" : : "r" (sel));
}

static inline uint
readeflags(void)
{
  uint eflags;
  asm volatile("pushfl; popl %0" : "=r" (eflags));
  return eflags;
}

static inline void
loadgs(ushort v)
{
  asm volatile("movw %0, %%gs" : : "r" (v));
}

static inline void
cli(void)
{
  asm volatile("cli");
}

static inline void
sti(void)
{
  asm volatile("sti");
}

static inline uint
xchg(volatile uint *addr, uint newval)
{
  uint result;

  // The + in "+m" denotes a read-modify-write operand.
  asm volatile("lock; xchgl %0, %1" :
               "+m" (*addr), "=a" (result) :
               "1" (newval) :
               "cc");
  return result;
}

static inline uint
rcr2(void)
{
  uint val;
  asm volatile("movl %%cr2,%0" : "=r" (val));
  return val;
}

static inline void
lcr3(uint val)
{
  asm volatile("movl %0,%%cr3" : : "r" (val));
}

//PAGEBREAK: 36
// Layout of the trap frame built on the stack by the
// hardware and by trapasm.S, and passed to trap().
struct trapframe {
  // registers as pushed by pusha
  uint edi;
  uint esi;
  uint ebp;
  uint oesp;      // useless & ignored
  uint ebx;
  uint edx;
  uint ecx;
  uint eax;

  // rest of trap frame
  ushort gs;
  ushort padding1;
  ushort fs;
  ushort padding2;
  ushort es;
  ushort padding3;
  ushort ds;
  ushort padding4;
  uint trapno;

  // below here defined by x86 hardware
  uint err;
  uint eip;
  ushort cs;
  ushort padding5;
  uint eflags;

  // below here only when crossing rings, such as from user to kernel
  uint esp;
  ushort ss;
  ushort padding6;
};
