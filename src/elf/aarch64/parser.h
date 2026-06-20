#pragma once

#include "elf_defs.h"
#include "../../common/types.h"

// ELF文件结构
typedef struct {
    // 原始数据
    uint8_t* data;
    size_t size;
    
    // 头部
    Elf64_Ehdr* ehdr;
    
    // 段头表
    Elf64_Shdr* shdrs;
    uint16_t shnum;
    
    // 程序头表
    Elf64_Phdr* phdrs;
    uint16_t phnum;
    
    // 字符串表
    char* shstrtab;    // 段名字符串表
    size_t shstrtab_size;
    
    // 符号表和字符串表
    Elf64_Sym* symtab;     // 静态符号表
    size_t symtab_num;
    char* strtab;          // 静态字符串表
    size_t strtab_size;
    
    Elf64_Sym* dynsym;     // 动态符号表
    size_t dynsym_num;
    char* dynstr;          // 动态字符串表
    size_t dynstr_size;
    
    // 动态段
    Elf64_Dyn* dynamic;
    size_t dynamic_num;
    
} elf_file_t;

// 段信息
typedef struct {
    Elf64_Shdr* shdr;
    char* name;
    uint8_t* data;
    size_t size;
    uint16_t index;
} section_info_t;

// API函数
result_t elf_parse(const char* filename, elf_file_t** elf_out);
void elf_free(elf_file_t* elf);

// 段查询
section_info_t* elf_find_section(elf_file_t* elf, const char* name);
section_info_t* elf_get_section(elf_file_t* elf, uint16_t index);
const char* elf_get_section_name(elf_file_t* elf, uint16_t index);
const char* elf_get_string(elf_file_t* elf, uint32_t index, bool is_dynamic);
const char* elf_get_symbol_name(elf_file_t* elf, Elf64_Sym* sym, bool is_dynamic);

// 验证
bool elf_is_valid(const uint8_t* data, size_t size);
bool elf_is_dynamic_lib(elf_file_t* elf);
bool elf_is_aarch64(elf_file_t* elf);

