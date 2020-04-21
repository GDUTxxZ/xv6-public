// Format of an ELF executable file

#define ELF_MAGIC 0x464C457FU  // "\x7FELF" in little endian

// e_type 目标文件类型
#define ET_NONE 0 // 未知目标文件格式
#define ET_REL 1 // 可重定向文件
#define ET_EXEC 2 // 可执行文件
#define ET_DYN 3 // 共享目标文件
#define ET_CORE 4 // core文件（转储格式）
#define ET_LOPROC 0xff00 // 0xff00特定处理器文件
#define ET_HIPROC 0xffff // 0xffff特别处理器文件
// e_machine
#define EM_NONE 0 // 未指定
#define EM_M32 1 // AT&T WE 32100
#define EM_SPARC 2 // SPARC
#define EM_386 3 // intel 80386
#define EM_68K 4 // Motorola 68000
#define EM_88K 5 // Motorola 88000
#define EM_860 7 // inter 80860
#define EM_MIPS 8 // MIPS RS3000

// File header
struct elfhdr {
  uint magic;  // must equal ELF_MAGIC
  uchar elf[12]; // 目标文件标志
  ushort type; // 目标文件类型，参考 e_type
  ushort machine; // 输出文件的目标体系结构, 参考 e_machine
  uint version; // 目标文件版本 EV_NONE 0 非法版本, EV_CURRENT 1 当前版本
  uint entry; // 程序入口的虚拟地址，如果没有可以为0
  uint phoff; // 程序头部表格的偏移量（按字节计算），没有可以为0
  uint shoff; // 节区的头部表哥的偏移量（按字节计算），没有可以为0
  uint flags; // 保存与文件相关的处理器标志，参考 e_machine
  ushort ehsize; // ELF头部大小（按字节计算）
  ushort phentsize; // 程序头部表格表项大小（按字节计算）
  ushort phnum; // 程序头部表格表项数目，可以为0
  ushort shentsize; // 节区头部表格表项大小（按字节计算）
  ushort shnum; // 节区头部表格表项数目，可以为0
  ushort shstrndx; // 节区头部表格中与节区名称字符串表相关的表项的索引。如果文件没有节区名称字符串表，吃参数可以为SHN_UNDEF 
};

// Program section header
//可执行文件或者共享目标文件的程序头部是一个结构数组，每个结构描述了一个段 或者系统准备程序执行所必需的其它信息。目标文件的“段”包含一个或者多个“节区”， 也就是“段内容(Segment Contents)”。程序头部仅对于可执行文件和共享目标文件 有意义。 可执行目标文件在 ELF 头部的 e_phentsize和e_phnum 成员中给出其自身程序头部 的大小。程序头部的数据结构:
struct proghdr {
  uint type; // 段的类型，或者如何解释此数组元素的信息。 
  uint off; // 此成员给出从文件头到该段第一个字节的偏移。 
  uint vaddr; // 此成员给出段的第一个字节将被放到内存中的虚拟地址。 
  uint paddr; // 此成员仅用于与物理地址相关的系统中。因为 System V 忽略所有应用程序的物理地址信息，此字段对与可执行文件和共享目标文件而言具体内容是指定的。 
  uint filesz; // 此成员给出段在文件映像中所占的字节数。可以为 0。
  uint memsz; // 此成员给出段在内存映像中占用的字节数。可以为 0。
  uint flags; // 此成员给出与段相关的标志。
  uint align; // 可加载的进程段的 p_vaddr 和 p_offset 取值必须合适，相对于对页面大小的取模而言。此成员给出段在文件中和内存中如何 对齐。数值 0 和 1 表示不需要对齐。否则 p_align 应该是个正整数，并且是 2 的幂次数，p_vaddr 和 p_offset 对 p_align 取模后应该相等。
};

// Values for Proghdr type
#define ELF_PROG_LOAD           1

// Flag bits for Proghdr flags
#define ELF_PROG_FLAG_EXEC      1
#define ELF_PROG_FLAG_WRITE     2
#define ELF_PROG_FLAG_READ      4
