#pragma once

#include "../elf/aarch64/elf_defs.h"
#include "../common/types.h"

// 重定位信息
typedef struct {
    Elf64_Rela* entries;   // 重定位表项
    size_t count;          // 数量
    uint16_t section_idx;  // 目标段索引
    char* name;            // 重定位段名
} reloc_table_t;

// 重定位集合
typedef struct {
    reloc_table_t* tables;  // 重定位表数组
    size_t count;           // 重定位表数量
} reloc_set_t;

// 从动态库提取重定位信息
result_t extract_relocations(void* elf_file, reloc_set_t** relocs_out);

// 转换动态重定位为静态重定位
result_t convert_dynamic_relocs(reloc_set_t* relocs);

// 释放重定位集合
void reloc_set_free(reloc_set_t* relocs);

