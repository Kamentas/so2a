#pragma once

#include "../common/types.h"
#include "../common/memory.h"
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

// 字符串表视图（零拷贝设计）
// 参考 rust-elf-master 的 StringTable 实现
typedef struct {
    const uint8_t* data;   // 原始数据（不拥有所有权）
    size_t size;           // 数据大小
} string_table_view_t;

// 创建字符串表视图
static inline result_t string_table_view_create(string_table_view_t* view, 
                                                 const void* data, 
                                                 size_t size) {
    if (!view || !data || size == 0) {
        return ERR(ERR_INVALID_ARGUMENT, "Invalid arguments for string table view");
    }
    
    view->data = (const uint8_t*)data;
    view->size = size;
    return OK();
}

// 获取原始字符串（返回字节切片，零拷贝）
static inline const uint8_t* string_table_get_raw(const string_table_view_t* view,
                                                   size_t offset,
                                                   size_t* out_len) {
    if (!view || !view->data || offset >= view->size) {
        return nullptr;
    }
    
    // 查找终止符\0
    size_t len = 0;
    while ((offset + len) < view->size && view->data[offset + len] != '\0') {
        len++;
    }
    
    // 检查是否找到终止符
    if ((offset + len) >= view->size) {
        return nullptr;  // 没有找到终止符
    }
    
    if (out_len) {
        *out_len = len;
    }
    
    return &view->data[offset];
}

// 获取字符串（返回 C 字符串，零拷贝）
static inline const char* string_table_get(const string_table_view_t* view,
                                           size_t offset) {
    if (!view || !view->data || offset >= view->size) {
        return nullptr;
    }
    
    // 检查是否是空字符串（偏移 0）
    if (offset == 0) {
        return "";
    }
    
    // 验证字符串有效性
    const uint8_t* str = &view->data[offset];
    size_t remaining = view->size - offset;
    
    // 确保有终止符
    bool has_nul = false;
    for (size_t i = 0; i < remaining; i++) {
        if (str[i] == '\0') {
            has_nul = true;
            break;
        }
    }
    
    if (!has_nul) {
        return nullptr;
    }
    
    return (const char*)str;
}

// 可写字符串表（用于构建新的 ELF 文件）
typedef struct {
    uint8_t* data;         // 数据缓冲区
    size_t size;           // 当前大小
    size_t capacity;       // 总容量
    bool owns_data;        // 是否拥有数据所有权
} string_table_t;

// 创建字符串表
static inline string_table_t* string_table_create(void) {
    string_table_t* strtab = (string_table_t*)malloc(sizeof(string_table_t));
    if (!strtab) {
        return nullptr;
    }
    
    strtab->capacity = 4096;  // 初始容量 4KB
    strtab->data = (uint8_t*)calloc(1, strtab->capacity);
    if (!strtab->data) {
        free(strtab);
        return nullptr;
    }
    
    strtab->size = 1;  // 第一个字节为\0
    strtab->owns_data = true;
    
    return strtab;
}

// 添加字符串并返回偏移量
static inline uint32_t string_table_add(string_table_t* strtab, const char* str) {
    if (!strtab || !strtab->data) {
        return 0;
    }
    
    // 空字符串返回偏移 0
    if (!str || str[0] == '\0') {
        return 0;
    }
    
    size_t len = strlen(str) + 1;  // 包括'\0'
    
    // 扩容检查
    while (strtab->size + len > strtab->capacity) {
        size_t new_capacity = strtab->capacity * 2;
        uint8_t* new_data = (uint8_t*)realloc(strtab->data, new_capacity);
        if (!new_data) {
            return 0;  // 分配失败
        }
        strtab->data = new_data;
        strtab->capacity = new_capacity;
    }
    
    uint32_t offset = strtab->size;
    memcpy(strtab->data + offset, str, len);
    strtab->size += len;
    
    return offset;
}

// 释放字符串表
static inline void string_table_free(string_table_t* strtab) {
    if (!strtab) return;
    
    if (strtab->owns_data && strtab->data) {
        free(strtab->data);
    }
    
    free(strtab);
}

// 从字符串表视图查找字符串（通过名称）
static inline int string_table_find_by_name(const string_table_view_t* view,
                                            const char* name,
                                            size_t start_offset) {
    if (!view || !name || start_offset >= view->size) {
        return -1;
    }
    
    size_t offset = start_offset;
    
    while (offset < view->size) {
        const char* str = string_table_get(view, offset);
        if (!str) {
            break;  // 到达末尾或错误
        }
        
        if (strcmp(str, name) == 0) {
            return (int)offset;
        }
        
        // 移动到下一个字符串
        offset += strlen(str) + 1;
    }
    
    return -1;  // 未找到
}

// 符号表项（简化版，用于解析）
typedef struct {
    uint32_t st_name;      // 符号名在字符串表中的偏移
    uint64_t st_value;     // 符号值
    uint64_t st_size;      // 符号大小
    uint8_t st_info;       // 符号类型和绑定
    uint8_t st_other;      // 符号可见性
    uint16_t st_shndx;     // 段索引
} elf_symbol_t;

// 符号表视图（零拷贝）
typedef struct {
    const elf_symbol_t* symbols;  // 符号数组
    size_t count;                  // 符号数量
    const string_table_view_t* strtab;  // 关联的字符串表
} symbol_table_view_t;

// 创建符号表视图
static inline result_t symbol_table_view_create(symbol_table_view_t* view,
                                                 const void* data,
                                                 size_t size,
                                                 const string_table_view_t* strtab) {
    if (!view || !data || !strtab) {
        return ERR(ERR_INVALID_ARGUMENT, "Invalid arguments for symbol table view");
    }
    
    // 检查大小是否合理
    if (size % sizeof(elf_symbol_t) != 0) {
        return ERR_MISMATCH(ERR_ELF_INVALID_SECTION,
                           "Symbol table size not aligned",
                           size,
                           sizeof(elf_symbol_t));
    }
    
    view->symbols = (const elf_symbol_t*)data;
    view->count = size / sizeof(elf_symbol_t);
    view->strtab = strtab;
    
    return OK();
}

// 获取符号
static inline const elf_symbol_t* symbol_table_get(const symbol_table_view_t* view,
                                                    size_t index) {
    if (!view || index >= view->count) {
        return nullptr;
    }
    return &view->symbols[index];
}

// 获取符号名称
static inline const char* symbol_table_get_name(const symbol_table_view_t* view,
                                                 size_t index) {
    if (!view || index >= view->count) {
        return nullptr;
    }
    
    const elf_symbol_t* sym = &view->symbols[index];
    if (!view->strtab) {
        return nullptr;
    }
    
    return string_table_get(view->strtab, sym->st_name);
}

// 辅助宏
#define ELF_ST_BIND(info)      ((info) >> 4)
#define ELF_ST_TYPE(info)      ((info) & 0xf)
#define ELF_ST_VISIBILITY(other) ((other) & 0x3)
