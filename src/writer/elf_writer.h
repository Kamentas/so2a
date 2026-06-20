#pragma once

#include "../elf/aarch64/elf_defs.h"
#include "../converter/section_builder.h"
#include "../converter/symbol_table.h"
#include "../converter/relocator.h"
#include "../common/types.h"

// ELF写入上下文
typedef struct {
    section_set_t* sections;    // 段集合
    symbol_table_t* symtab;     // 符号表
    reloc_set_t* relocs;        // 重定位表
    string_table_t* shstrtab;   // 段名字符串表
    string_table_t* strtab;     // 符号字符串表
    
    uint8_t* buffer;            // 输出缓冲区
    size_t buffer_size;         // 缓冲区大小
    size_t buffer_offset;       // 当前写入位置
} elf_writer_t;

// 创建ELF写入器
elf_writer_t* elf_writer_create(section_set_t* sections, 
                                 symbol_table_t* symtab,
                                 reloc_set_t* relocs);

// 写入可重定位ELF对象文件
result_t elf_writer_write(elf_writer_t* writer, const char* output_path);

// 释放写入器
void elf_writer_free(elf_writer_t* writer);

