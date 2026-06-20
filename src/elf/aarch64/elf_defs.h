#pragma once

#include "../../common/types.h"

// ELF魔数
#define ELF_MAGIC "\x7f""ELF"
#define EI_NIDENT 16

// ELF类别
#define ELFCLASSNONE 0
#define ELFCLASS32   1
#define ELFCLASS64   2

// 数据编码
#define ELFDATANONE 0
#define ELFDATA2LSB 1  // 小端
#define ELFDATA2MSB 2  // 大端

// ELF版本
#define EV_NONE    0
#define EV_CURRENT 1

// 文件类型
#define ET_NONE   0  // 无类型
#define ET_REL    1  // 可重定位文件
#define ET_EXEC   2  // 可执行文件
#define ET_DYN    3  // 共享对象文件
#define ET_CORE   4  // Core文件

// 机器架构
#define EM_NONE    0
#define EM_X86_64  62
#define EM_AARCH64 183

// ELF段类型
#define SHT_NULL          0   // 无效段
#define SHT_PROGBITS      1   // 程序数据
#define SHT_SYMTAB        2   // 符号表
#define SHT_STRTAB        3   // 字符串表
#define SHT_RELA          4   // 重定位表(带附加)
#define SHT_HASH          5   // 符号哈希表
#define SHT_DYNAMIC       6   // 动态链接信息
#define SHT_NOTE          7   // 注释信息
#define SHT_NOBITS        8   // 不占文件空间
#define SHT_REL           9   // 重定位表
#define SHT_SHLIB         10  // 保留
#define SHT_DYNSYM        11  // 动态符号表
#define SHT_INIT_ARRAY    14  // 构造函数数组
#define SHT_FINI_ARRAY    15  // 析构函数数组
#define SHT_PREINIT_ARRAY 16  // 预构造函数数组
#define SHT_GROUP         17  // 段组
#define SHT_SYMTAB_SHNDX  18  // 扩展段索引

// 段标志
#define SHF_WRITE            (1 << 0)  // 可写
#define SHF_ALLOC            (1 << 1)  // 在内存中分配
#define SHF_EXECINSTR        (1 << 2)  // 可执行
#define SHF_MERGE            (1 << 4)  // 可合并
#define SHF_STRINGS          (1 << 5)  // 包含字符串
#define SHF_INFO_LINK        (1 << 6)  // sh_info包含段索引
#define SHF_LINK_ORDER       (1 << 7)  // 保持链接顺序
#define SHF_OS_NONCONFORMING (1 << 8)  // 特殊OS处理
#define SHF_GROUP            (1 << 9)  // 段组成员
#define SHF_TLS              (1 << 10) // 线程本地存储

// 特殊段索引
#define SHN_UNDEF     0
#define SHN_ABS       0xfff1
#define SHN_COMMON    0xfff2

// 程序头类型
#define PT_NULL    0
#define PT_LOAD    1
#define PT_DYNAMIC 2
#define PT_INTERP  3
#define PT_NOTE    4
#define PT_SHLIB   5
#define PT_PHDR    6
#define PT_TLS     7

// 动态段标签
#define DT_NULL            0
#define DT_NEEDED          1
#define DT_PLTRELSZ        2
#define DT_PLTGOT          3
#define DT_HASH            4
#define DT_STRTAB          5
#define DT_SYMTAB          6
#define DT_RELA            7
#define DT_RELASZ          8
#define DT_RELAENT         9
#define DT_STRSZ           10
#define DT_SYMENT          11
#define DT_INIT            12
#define DT_FINI            13
#define DT_SONAME          14
#define DT_RPATH           15
#define DT_SYMBOLIC        16
#define DT_REL             17
#define DT_RELSZ           18
#define DT_RELENT          19
#define DT_PLTREL          20
#define DT_DEBUG           21
#define DT_TEXTREL         22
#define DT_JMPREL          23
#define DT_BIND_NOW        24
#define DT_INIT_ARRAY      25
#define DT_FINI_ARRAY      26
#define DT_INIT_ARRAYSZ    27
#define DT_FINI_ARRAYSZ    28

// 符号绑定
#define STB_LOCAL  0
#define STB_GLOBAL 1
#define STB_WEAK   2

// 符号类型
#define STT_NOTYPE  0
#define STT_OBJECT  1
#define STT_FUNC    2
#define STT_SECTION 3
#define STT_FILE    4
#define STT_COMMON  5
#define STT_TLS     6

// 符号可见性
#define STV_DEFAULT   0
#define STV_INTERNAL  1
#define STV_HIDDEN    2
#define STV_PROTECTED 3

// 符号辅助宏
#define ELF64_ST_BIND(i)   ((i) >> 4)
#define ELF64_ST_TYPE(i)   ((i) & 0xf)
#define ELF64_ST_INFO(b,t) (((b) << 4) + ((t) & 0xf))
#define ELF64_ST_VISIBILITY(o) ((o) & 0x3)

// 重定位宏
#define ELF64_R_SYM(i)    ((i) >> 32)
#define ELF64_R_TYPE(i)   ((i) & 0xffffffffL)
#define ELF64_R_INFO(s,t) ((((uint64_t)(s)) << 32) + ((t) & 0xffffffffL))

// AArch64重定位类型
#define R_AARCH64_NONE                  0
#define R_AARCH64_ABS64                 257
#define R_AARCH64_ABS32                 258
#define R_AARCH64_ABS16                 259
#define R_AARCH64_PREL64                260
#define R_AARCH64_PREL32                261
#define R_AARCH64_PREL16                262
#define R_AARCH64_MOVW_UABS_G0          263
#define R_AARCH64_MOVW_UABS_G0_NC       264
#define R_AARCH64_MOVW_UABS_G1          265
#define R_AARCH64_MOVW_UABS_G1_NC       266
#define R_AARCH64_MOVW_UABS_G2          267
#define R_AARCH64_MOVW_UABS_G2_NC       268
#define R_AARCH64_MOVW_UABS_G3          269
#define R_AARCH64_ADR_PREL_LO21         274
#define R_AARCH64_ADR_PREL_PG_HI21      275
#define R_AARCH64_ADR_PREL_PG_HI21_NC   276
#define R_AARCH64_ADD_ABS_LO12_NC       277
#define R_AARCH64_LDST8_ABS_LO12_NC     278
#define R_AARCH64_LDST16_ABS_LO12_NC    284
#define R_AARCH64_LDST32_ABS_LO12_NC    285
#define R_AARCH64_LDST64_ABS_LO12_NC    286
#define R_AARCH64_LDST128_ABS_LO12_NC   299
#define R_AARCH64_CALL26                283
#define R_AARCH64_JUMP26                282
#define R_AARCH64_GLOB_DAT              1025
#define R_AARCH64_JUMP_SLOT             1026
#define R_AARCH64_RELATIVE              1027
#define R_AARCH64_TLS_DTPREL64          1028
#define R_AARCH64_TLS_DTPMOD64          1029
#define R_AARCH64_TLS_TPREL64           1030
#define R_AARCH64_TLSDESC              1031
#define R_AARCH64_IRELATIVE             1032

// ELF文件头
typedef struct {
    unsigned char e_ident[EI_NIDENT];  // 魔数和其他信息
    Elf64_Half    e_type;              // 文件类型
    Elf64_Half    e_machine;           // 架构
    Elf64_Word    e_version;           // 版本
    Elf64_Addr    e_entry;             // 入口点地址
    Elf64_Off     e_phoff;             // 程序头表偏移
    Elf64_Off     e_shoff;             // 段头表偏移
    Elf64_Word    e_flags;             // 处理器特定标志
    Elf64_Half    e_ehsize;            // ELF头大小
    Elf64_Half    e_phentsize;         // 程序头表项大小
    Elf64_Half    e_phnum;             // 程序头表项数量
    Elf64_Half    e_shentsize;         // 段头表项大小
    Elf64_Half    e_shnum;             // 段头表项数量
    Elf64_Half    e_shstrndx;          // 段名字符串表索引
} Elf64_Ehdr;

// 段头表
typedef struct {
    Elf64_Word  sh_name;       // 段名（字符串表索引）
    Elf64_Word  sh_type;       // 段类型
    Elf64_Xword sh_flags;      // 段标志
    Elf64_Addr  sh_addr;       // 内存中的地址
    Elf64_Off   sh_offset;     // 文件偏移
    Elf64_Xword sh_size;       // 段大小
    Elf64_Word  sh_link;       // 链接到其他段
    Elf64_Word  sh_info;       // 附加信息
    Elf64_Xword sh_addralign;  // 对齐
    Elf64_Xword sh_entsize;    // 表项大小（如果段包含表）
} Elf64_Shdr;

// 程序头表
typedef struct {
    Elf64_Word  p_type;    // 段类型
    Elf64_Word  p_flags;   // 段标志
    Elf64_Off   p_offset;  // 文件偏移
    Elf64_Addr  p_vaddr;   // 虚拟地址
    Elf64_Addr  p_paddr;   // 物理地址
    Elf64_Xword p_filesz;  // 文件中大小
    Elf64_Xword p_memsz;   // 内存中大小
    Elf64_Xword p_align;   // 对齐
} Elf64_Phdr;

// 符号表项
typedef struct {
    Elf64_Word    st_name;  // 符号名（字符串表索引）
    unsigned char st_info;  // 类型和绑定属性
    unsigned char st_other; // 可见性
    Elf64_Half    st_shndx; // 段索引
    Elf64_Addr    st_value; // 符号值
    Elf64_Xword   st_size;  // 符号大小
} Elf64_Sym;

// 重定位表项（带附加）
typedef struct {
    Elf64_Addr   r_offset; // 重定位位置
    Elf64_Xword  r_info;   // 符号索引和类型
    Elf64_Sxword r_addend; // 常量附加值
} Elf64_Rela;

// 重定位表项（不带附加）
typedef struct {
    Elf64_Addr  r_offset; // 重定位位置
    Elf64_Xword r_info;   // 符号索引和类型
} Elf64_Rel;

// 动态段
typedef struct {
    Elf64_Sxword d_tag;  // 条目类型
    union {
        Elf64_Xword d_val;  // 整数值
        Elf64_Addr  d_ptr;  // 地址值
    } d_un;
} Elf64_Dyn;

