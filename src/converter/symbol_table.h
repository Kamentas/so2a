#pragma once

#include "../elf/aarch64/elf_defs.h"
#include "../common/types.h"

// 符号条目
typedef struct {
    char* name;           // 符号名
    Elf64_Addr value;     // 符号值
    Elf64_Xword size;     // 符号大小
    unsigned char bind;   // 绑定属性
    unsigned char type;   // 符号类型
    unsigned char visibility; // 可见性
    uint16_t section_idx; // 段索引
} symbol_entry_t;

// 符号表
typedef struct {
    symbol_entry_t* entries;  // 符号数组
    size_t count;             // 符号数量
    size_t capacity;          // 容量
} symbol_table_t;

// 字符串表
typedef struct {
    char* data;      // 字符串数据
    size_t size;     // 当前大小
    size_t capacity; // 容量
} string_table_t;

// 创建符号表
symbol_table_t* symbol_table_create(void);

// 添加符号
uint32_t symbol_table_add(symbol_table_t* symtab, const symbol_entry_t* entry);

// 从ELF提取符号（合并静态和动态符号）
result_t extract_symbols(void* elf_file, symbol_table_t** symtab_out);

// 转换动态符号为静态符号
result_t convert_dynamic_symbols(symbol_table_t* symtab);

// 释放符号表
void symbol_table_free(symbol_table_t* symtab);

// 字符串表操作
string_table_t* string_table_create(void);
uint32_t string_table_add(string_table_t* strtab, const char* str);
void string_table_free(string_table_t* strtab);

