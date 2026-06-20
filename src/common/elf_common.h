#pragma once

#include <stdint.h>

// ELF 魔数
#define ELF_MAGIC "\x7fELF"
#define ELF_MAG0  0x7f
#define ELF_MAG1  0x45
#define ELF_MAG2  0x4c
#define ELF_MAG3  0x46

// EI_IDENT 数组长度
#define EI_NIDENT 16

// e_ident 索引
#define EI_CLASS    4   // 文件类别
#define EI_DATA     5   // 数据编码（端序）
#define EI_VERSION  6   // ELF 版本
#define EI_OSABI    7   // OS ABI
#define EI_ABIVERSION 8 // ABI 版本

// ELF 类别
#define ELFCLASSNONE 0  // 无效类别
#define ELFCLASS32   1  // 32 位对象
#define ELFCLASS64   2  // 64 位对象

// 数据编码（端序）
#define ELFDATANONE 0   // 无效数据编码
#define ELFDATA2LSB 1   // 小端序
#define ELFDATA2MSB 2   // 大端序

// ELF 版本
#define EV_NONE    0
#define EV_CURRENT 1

// OS ABI
#define ELFOSABI_NONE      0   // UNIX System V
#define ELFOSABI_LINUX     3   // Linux
#define ELFOSABI_FREEBSD   9   // FreeBSD

// ELF 文件类型
#define ET_NONE   0     // 无文件类型
#define ET_REL    1     // 可重定位文件
#define ET_EXEC   2     // 可执行文件
#define ET_DYN    3     // 共享目标文件
#define ET_CORE   4     // Core 文件

// 机器类型
#define EM_NONE        0   // 无机器类型
#define EM_386         3   // Intel 80386
#define EM_ARM        40   // ARM
#define EM_X86_64     62   // AMD x86-64
#define EM_AARCH64   183   // ARM AArch64
#define EM_RISCV     243   // RISC-V

// 段类型
#define SHT_NULL          0  // 非活动段头
#define SHT_PROGBITS      1  // 程序信息
#define SHT_SYMTAB        2  // 符号表
#define SHT_STRTAB        3  // 字符串表
#define SHT_RELA          4  // 带加数的重定位项
#define SHT_HASH          5  // 符号哈希表
#define SHT_DYNAMIC       6  // 动态链接信息
#define SHT_NOTE          7  // 注释信息
#define SHT_NOBITS        8  // 未初始化数据（.bss）
#define SHT_REL           9  // 重定位项（无加数）
#define SHT_DYNSYM       11  // 动态符号表
#define SHT_INIT_ARRAY    14  // 初始化函数指针数组
#define SHT_FINI_ARRAY    15  // 终止函数指针数组
#define SHT_GNU_HASH    0x6ffffff6  // GNU 风格哈希表
#define SHT_GNU_VERSYM  0x6fffffff  // 版本符号表
#define SHT_GNU_VERNEED 0x6ffffffe  // 版本需求表

// 段标志
#define SHF_WRITE            0x1  // 可写
#define SHF_ALLOC            0x2  // 占用内存
#define SHF_EXECINSTR        0x4  // 包含可执行代码
#define SHF_MERGE            0x10 // 可以合并
#define SHF_STRINGS          0x20 // 以 null 结尾的字符串
#define SHF_INFO_LINK        0x40 // sh_info 包含索引
#define SHF_TLS              0x400 // 线程局部存储
#define SHF_COMPRESSED       0x800 // 压缩段

// 特殊段索引
#define SHN_UNDEF     0     // 未定义符号
#define SHN_LORESERVE 0xff00 // 保留范围开始
#define SHN_ABS      0xfff1 // 绝对符号
#define SHN_COMMON   0xfff2 // 公共符号
#define SHN_XINDEX   0xffff // 扩展索引

// 符号绑定
#define STB_LOCAL    0  // 局部符号
#define STB_GLOBAL   1  // 全局符号
#define STB_WEAK     2  // 弱符号

// 符号类型
#define STT_NOTYPE   0  // 无类型
#define STT_OBJECT   1  // 数据对象
#define STT_FUNC     2  // 函数
#define STT_SECTION  3  // 段
#define STT_FILE     4  // 文件
#define STT_COMMON   5  // 未分配的数据项
#define STT_TLS      6  // 线程局部存储对象

// 符号可见性
#define STV_DEFAULT   0  // 默认可见性
#define STV_INTERNAL  1  // 内部使用
#define STV_HIDDEN    2  // 隐藏
#define STV_PROTECTED 3  // 受保护

// 程序头类型
#define PT_NULL         0  // 未使用
#define PT_LOAD         1  // 可加载段
#define PT_DYNAMIC      2  // 动态链接信息
#define PT_INTERP       3  // 解释器路径
#define PT_NOTE         4  // 注释信息
#define PT_PHDR         6  // 程序头表本身
#define PT_GNU_EH_FRAME 0x6474e550
#define PT_GNU_STACK    0x6474e551
#define PT_GNU_RELRO    0x6474e552

// 程序头标志
#define PF_X  0x1  // 可执行
#define PF_W  0x2  // 可写
#define PF_R  0x4  // 可读

// 动态标记
#define DT_NULL            0  // 数组结束
#define DT_NEEDED          1  // 依赖的共享库
#define DT_PLTRELSZ        2  // 重定位表大小
#define DT_PLTGOT          3  // PLT/GOT 地址
#define DT_HASH            4  // 符号哈希表地址
#define DT_STRTAB          5  // 字符串表地址
#define DT_SYMTAB          6  // 符号表地址
#define DT_RELA            7  // 重定位表地址
#define DT_RELASZ          8  // 重定位表大小
#define DT_RELAENT         9  // 重定位项大小
#define DT_STRSZ          10  // 字符串表大小
#define DT_SYMENT         11  // 符号表项大小
#define DT_INIT           12  // 初始化函数地址
#define DT_FINI           13  // 终止函数地址
#define DT_SONAME         14  // 共享库名称
#define DT_RPATH          15  // 运行时搜索路径
#define DT_SYMBOLIC       16  // 符号解析范围
#define DT_REL            17  // 重定位表地址（无加数）
#define DT_RELSZ          18  // 重定位表大小（无加数）
#define DT_RELENT         19  // 重定位项大小（无加数）
#define DT_BIND_NOW       24  // 立即绑定所有符号
#define DT_INIT_ARRAY     25  // 初始化函数数组地址
#define DT_FINI_ARRAY     26  // 终止函数数组地址
#define DT_RUNPATH        29  // 运行时搜索路径
#define DT_FLAGS          30  // 标志
#define DT_VERSYM         0x6ffffff0  // 版本符号表
#define DT_VERDEF         0x6ffffffc  // 版本定义表
#define DT_VERNEED        0x6ffffffd  // 版本需求表
#define DT_VERNEEDNUM     0x6fffffff  // 版本需求表项数

// 重定位类型 (AArch64)
#define R_AARCH64_NONE                0
#define R_AARCH64_ABS64              257
#define R_AARCH64_ABS32              258
#define R_AARCH64_ABS16              259
#define R_AARCH64_PREL64             260
#define R_AARCH64_PREL32             261
#define R_AARCH64_PREL16             262
#define R_AARCH64_MOVW_UABS_G0       263
#define R_AARCH64_MOVW_UABS_G0_NC    264
#define R_AARCH64_MOVW_UABS_G1       265
#define R_AARCH64_MOVW_UABS_G1_NC    266
#define R_AARCH64_MOVW_UABS_G2       267
#define R_AARCH64_MOVW_UABS_G2_NC    268
#define R_AARCH64_MOVW_UABS_G3       269
#define R_AARCH64_MOVW_SABS_G0       270
#define R_AARCH64_MOVW_SABS_G1       271
#define R_AARCH64_MOVW_SABS_G2       272
#define R_AARCH64_LD_PREL_LO19       273
#define R_AARCH64_ADR_PREL_LO21      274
#define R_AARCH64_ADR_PREL_PG_HI21   275
#define R_AARCH64_ADD_ABS_LO12_NC    277
#define R_AARCH64_CALL26             283
#define R_AARCH64_JUMP26             284

// ELF64 基本结构
typedef uint64_t Elf64_Addr;
typedef uint64_t Elf64_Off;
typedef uint16_t Elf64_Half;
typedef uint32_t Elf64_Word;
typedef int32_t  Elf64_Sword;
typedef uint64_t Elf64_Xword;
typedef int64_t  Elf64_Sxword;

// ELF64 文件头
typedef struct {
    unsigned char e_ident[EI_NIDENT];
    Elf64_Half    e_type;
    Elf64_Half    e_machine;
    Elf64_Word    e_version;
    Elf64_Addr    e_entry;
    Elf64_Off     e_phoff;
    Elf64_Off     e_shoff;
    Elf64_Word    e_flags;
    Elf64_Half    e_ehsize;
    Elf64_Half    e_phentsize;
    Elf64_Half    e_phnum;
    Elf64_Half    e_shentsize;
    Elf64_Half    e_shnum;
    Elf64_Half    e_shstrndx;
} Elf64_Ehdr;

// ELF64 段头
typedef struct {
    Elf64_Word  sh_name;
    Elf64_Word  sh_type;
    Elf64_Xword sh_flags;
    Elf64_Addr  sh_addr;
    Elf64_Off   sh_offset;
    Elf64_Xword sh_size;
    Elf64_Word  sh_link;
    Elf64_Word  sh_info;
    Elf64_Xword sh_addralign;
    Elf64_Xword sh_entsize;
} Elf64_Shdr;

// ELF64 程序头
typedef struct {
    Elf64_Word  p_type;
    Elf64_Word  p_flags;
    Elf64_Off   p_offset;
    Elf64_Addr  p_vaddr;
    Elf64_Addr  p_paddr;
    Elf64_Xword p_filesz;
    Elf64_Xword p_memsz;
    Elf64_Xword p_align;
} Elf64_Phdr;

// ELF64 符号表项
typedef struct {
    Elf64_Word  st_name;
    unsigned char st_info;
    unsigned char st_other;
    Elf64_Half  st_shndx;
    Elf64_Addr  st_value;
    Elf64_Xword st_size;
} Elf64_Sym;

// ELF64 重定位项（无加数）
typedef struct {
    Elf64_Addr  r_offset;
    Elf64_Xword r_info;
} Elf64_Rel;

// ELF64 重定位项（有加数）
typedef struct {
    Elf64_Addr  r_offset;
    Elf64_Sxword r_addend;
    Elf64_Xword r_info;
} Elf64_Rela;

// ELF64 动态项
typedef struct {
    Elf64_Sxword d_tag;
    union {
        Elf64_Xword d_val;
        Elf64_Addr  d_ptr;
    } d_un;
} Elf64_Dyn;

// 辅助宏
#define ELF64_ST_BIND(info)      ((info) >> 4)
#define ELF64_ST_TYPE(info)      ((info) & 0xf)
#define ELF64_ST_INFO(bind, type) (((bind) << 4) + ((type) & 0xf))
#define ELF64_ST_VISIBILITY(other) ((other) & 0x3)

#define ELF64_R_SYM(info) ((info) >> 32)
#define ELF64_R_TYPE(info) ((info) & 0xffffffff)
#define ELF64_R_INFO(sym, type) ((((Elf64_Xword)(sym)) << 32) + ((type) & 0xffffffff))
