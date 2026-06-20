#include "symbol_table.h"
#include "../elf/aarch64/parser.h"
#include "../common/utils.h"
#include <string.h>

// 创建符号表
symbol_table_t* symbol_table_create(void) {
    symbol_table_t* symtab = xcalloc(1, sizeof(symbol_table_t));
    symtab->capacity = 256;
    symtab->entries = xcalloc(symtab->capacity, sizeof(symbol_entry_t));
    symtab->count = 0;
    
    // 添加空符号（索引0）
    symbol_entry_t null_sym = {0};
    symbol_table_add(symtab, &null_sym);
    
    return symtab;
}

// 添加符号
uint32_t symbol_table_add(symbol_table_t* symtab, const symbol_entry_t* entry) {
    // 扩容检查
    if (symtab->count >= symtab->capacity) {
        symtab->capacity *= 2;
        symtab->entries = xrealloc(symtab->entries, 
                                   symtab->capacity * sizeof(symbol_entry_t));
    }
    
    uint32_t index = symtab->count++;
    symbol_entry_t* sym = &symtab->entries[index];
    
    sym->name = entry->name ? xstrdup(entry->name) : nullptr;
    sym->value = entry->value;
    sym->size = entry->size;
    sym->bind = entry->bind;
    sym->type = entry->type;
    sym->visibility = entry->visibility;
    sym->section_idx = entry->section_idx;
    
    return index;
}

// 从ELF符号提取
static void extract_elf_symbols(elf_file_t* elf, Elf64_Sym* syms, size_t count,
                                bool is_dynamic, symbol_table_t* symtab) {
    for (size_t i = 0; i < count; i++) {
        Elf64_Sym* sym = &syms[i];
        
        // 跳过空符号
        if (i == 0) continue;
        
        // 跳过未定义的符号
        if (sym->st_shndx == SHN_UNDEF) continue;
        
        const char* sym_name = elf_get_symbol_name(elf, sym, is_dynamic);
        if (!sym_name || sym_name[0] == '\0') continue;
        
        symbol_entry_t entry = {0};
        entry.name = xstrdup(sym_name);
        entry.value = sym->st_value;
        entry.size = sym->st_size;
        entry.bind = ELF64_ST_BIND(sym->st_info);
        entry.type = ELF64_ST_TYPE(sym->st_info);
        entry.visibility = ELF64_ST_VISIBILITY(sym->st_other);
        entry.section_idx = sym->st_shndx;
        
        symbol_table_add(symtab, &entry);
    }
}

// 从ELF提取符号
result_t extract_symbols(void* elf_file, symbol_table_t** symtab_out) {
    elf_file_t* elf = (elf_file_t*)elf_file;
    
    symbol_table_t* symtab = symbol_table_create();
    
    // 提取静态符号表
    if (elf->symtab && elf->symtab_num > 0) {
        extract_elf_symbols(elf, elf->symtab, elf->symtab_num, false, symtab);
    }
    
    // 提取动态符号表
    if (elf->dynsym && elf->dynsym_num > 0) {
        extract_elf_symbols(elf, elf->dynsym, elf->dynsym_num, true, symtab);
    }
    
    *symtab_out = symtab;
    return OK();
}

// 转换动态符号为静态符号
result_t convert_dynamic_symbols(symbol_table_t* symtab) {
    if (!symtab) return OK();
    
    // 动态符号通常标记为GLOBAL，在静态库中保持
    // 主要工作是确保符号绑定和可见性适合静态链接
    for (size_t i = 1; i < symtab->count; i++) {
        symbol_entry_t* sym = &symtab->entries[i];
        
        // 对于弱符号和全局符号，保持其绑定属性
        // 对于局部符号，保持局部
        // 动态符号的可见性转换为默认
        if (sym->visibility == STV_HIDDEN || sym->visibility == STV_INTERNAL) {
            sym->visibility = STV_DEFAULT;
        }
    }
    
    return OK();
}

// 释放符号表
void symbol_table_free(symbol_table_t* symtab) {
    if (!symtab) return;
    
    for (size_t i = 0; i < symtab->count; i++) {
        SAFE_FREE(symtab->entries[i].name);
    }
    
    SAFE_FREE(symtab->entries);
    free(symtab);
}

// 创建字符串表
string_table_t* string_table_create(void) {
    string_table_t* strtab = xcalloc(1, sizeof(string_table_t));
    strtab->capacity = 4096;
    strtab->data = xcalloc(strtab->capacity, 1);
    strtab->size = 1;  // 第一个字节为\0
    return strtab;
}

// 添加字符串
uint32_t string_table_add(string_table_t* strtab, const char* str) {
    if (!str || str[0] == '\0') {
        return 0;  // 空字符串
    }
    
    size_t len = strlen(str) + 1;  // 包括'\0'
    
    // 扩容检查
    while (strtab->size + len > strtab->capacity) {
        strtab->capacity *= 2;
        strtab->data = xrealloc(strtab->data, strtab->capacity);
    }
    
    uint32_t offset = strtab->size;
    memcpy(strtab->data + offset, str, len);
    strtab->size += len;
    
    return offset;
}

// 释放字符串表
void string_table_free(string_table_t* strtab) {
    if (!strtab) return;
    SAFE_FREE(strtab->data);
    free(strtab);
}

