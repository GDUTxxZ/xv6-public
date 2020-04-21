//
// assembler macros to create x86 segments
//

/**
 * define AC_AC 0x1       // 可访问 access
 * define AC_RW 0x2       // [代码]可读；[数据]可写 readable for code selector & writeable for data selector
 * define AC_DC 0x4       // 方向位 direction
 * define AC_EX 0x8       // 可执行 executable, code segment
 * define AC_RE 0x10      // 保留位 reserve
 * define AC_PR 0x80      // 有效位 persent in memory
 * /

#define SEG_NULLASM                                             \
        .word 0, 0;                                             \
        .byte 0, 0, 0, 0

// The 0xC0 means the limit is in 4096-byte units
// and (for executable segments) 32-bit mode.
#define SEG_ASM(type,base,lim)                                  \
        .word (((lim) >> 12) & 0xffff), ((base) & 0xffff);      \
        .byte (((base) >> 16) & 0xff), (0x90 | (type)),         \
                (0xC0 | (((lim) >> 28) & 0xf)), (((base) >> 24) & 0xff)

#define STA_X     0x8       // Executable segment
#define STA_W     0x2       // Writeable (non-executable segments)
#define STA_R     0x2       // Readable (executable segments)
