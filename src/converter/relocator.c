#include "relocator.h"
#include "../elf/aarch64/parser.h"
#include "../common/utils.h"
#include <string.h>

// 提取重定位信息
result_t extract_relocations(void* elf_file, reloc_set_t** relocs_out) {
    elf_file_t* elf = (elf_file_t*)elf_file;
    
    // 统计重定位段数量
    size_t reloc_count = 0;
    for (uint16_t i = 0; i < elf->shnum; i++) {
        Elf64_Shdr* shdr = &elf->shdrs[i];
        if (shdr->sh_type == SHT_RELA || shdr->sh_type == SHT_REL) {
            reloc_count++;
        }
    }
    
    if (reloc_count == 0) {
        // 没有重定位表，创建空集合
        reloc_set_t* relocs = xcalloc(1, sizeof(reloc_set_t));
        *relocs_out = relocs;
        return OK();
    }
    
    // 创建重定位集合
    reloc_set_t* relocs = xcalloc(1, sizeof(reloc_set_t));
    relocs->tables = xcalloc(reloc_count, sizeof(reloc_table_t));
    relocs->count = 0;
    
    // 提取每个重定位段
    for (uint16_t i = 0; i < elf->shnum; i++) {
        Elf64_Shdr* shdr = &elf->shdrs[i];
        
        if (shdr->sh_type == SHT_RELA) {
            Elf64_Rela* rela_entries = (Elf64_Rela*)(elf->data + shdr->sh_offset);
            size_t count = shdr->sh_size / sizeof(Elf64_Rela);
            
            reloc_table_t* table = &relocs->tables[relocs->count++];
            // 复制重定位表项，以便后续修改
            table->entries = xcalloc(count, sizeof(Elf64_Rela));
            memcpy(table->entries, rela_entries, count * sizeof(Elf64_Rela));
            table->count = count;
            table->section_idx = shdr->sh_info;
            table->name = xstrdup(elf_get_section_name(elf, i));
        }
        else if (shdr->sh_type == SHT_REL) {
            // REL类型需要转换为RELA
            Elf64_Rel* rel_entries = (Elf64_Rel*)(elf->data + shdr->sh_offset);
            size_t count = shdr->sh_size / sizeof(Elf64_Rel);
            
            reloc_table_t* table = &relocs->tables[relocs->count++];
            table->entries = xcalloc(count, sizeof(Elf64_Rela));
            table->count = count;
            table->section_idx = shdr->sh_info;
            table->name = xstrdup(elf_get_section_name(elf, i));
            
            // 转换REL到RELA
            for (size_t j = 0; j < count; j++) {
                table->entries[j].r_offset = rel_entries[j].r_offset;
                table->entries[j].r_info = rel_entries[j].r_info;
                table->entries[j].r_addend = 0;  // REL没有addend
            }
        }
    }
    
    *relocs_out = relocs;
    return OK();
}

// 转换动态重定位类型为静态重定位类型
static uint32_t convert_reloc_type(uint32_t dyn_type) {
    // 对于AArch64，大部分动态重定位类型可以直接转换
    switch (dyn_type) {
        case R_AARCH64_GLOB_DAT:
            return R_AARCH64_ABS64;
        case R_AARCH64_JUMP_SLOT:
            return R_AARCH64_CALL26;
        case R_AARCH64_RELATIVE:
            return R_AARCH64_PREL64;
        default:
            return dyn_type;  // 保持原类型
    }
}

// 转换动态重定位为静态重定位
result_t convert_dynamic_relocs(reloc_set_t* relocs) {
    if (!relocs) return OK();
    
    for (size_t i = 0; i < relocs->count; i++) {
        reloc_table_t* table = &relocs->tables[i];
        
        for (size_t j = 0; j < table->count; j++) {
            Elf64_Rela* rela = &table->entries[j];
            uint32_t sym = ELF64_R_SYM(rela->r_info);
            uint32_t type = ELF64_R_TYPE(rela->r_info);
            
            // 转换类型
            uint32_t new_type = convert_reloc_type(type);
            rela->r_info = ELF64_R_INFO(sym, new_type);
        }
    }
    
    return OK();
}

// 释放重定位集合
void reloc_set_free(reloc_set_t* relocs) {
    if (!relocs) return;
    
    for (size_t i = 0; i < relocs->count; i++) {
        SAFE_FREE(relocs->tables[i].name);
        SAFE_FREE(relocs->tables[i].entries);
    }
    
    SAFE_FREE(relocs->tables);
    free(relocs);
}

