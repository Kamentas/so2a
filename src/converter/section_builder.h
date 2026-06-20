#pragma once

#include "../elf/aarch64/elf_defs.h"
#include "../common/types.h"
#include "symbol_table.h"
#include "relocator.h"

// 段构建器
typedef struct {
    char* name;           // 段名
    uint32_t type;        // 段类型
    uint64_t flags;       // 段标志
    uint64_t addr;        // 地址
    uint64_t align;       // 对齐
    uint8_t* data;        // 段数据
    size_t size;          // 数据大小
    uint32_t link;        // 链接
    uint32_t info;        // 信息
    uint64_t entsize;     // 表项大小
} section_builder_t;

// 段集合
typedef struct {
    section_builder_t* sections;  // 段数组
    size_t count;                 // 段数量
    size_t capacity;              // 容量
} section_set_t;

// 创建段集合
section_set_t* section_set_create(void);

// 添加段
uint16_t section_set_add(section_set_t* set, section_builder_t* section);

// 从动态库提取段
result_t extract_sections(void* elf_file, section_set_t** sections_out);

// 转换段属性（动态->静态）
result_t convert_sections(section_set_t* sections);

// 释放段集合
void section_set_free(section_set_t* sections);

