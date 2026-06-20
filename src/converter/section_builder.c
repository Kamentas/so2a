#include "section_builder.h"
#include "../elf/aarch64/parser.h"
#include "../common/utils.h"
#include <string.h>

// 创建段集合
section_set_t* section_set_create(void) {
    section_set_t* set = xcalloc(1, sizeof(section_set_t));
    set->capacity = 64;
    set->sections = xcalloc(set->capacity, sizeof(section_builder_t));
    set->count = 0;
    
    // 添加NULL段（索引0）
    section_builder_t null_sec = {0};
    section_set_add(set, &null_sec);
    
    return set;
}

// 添加段
uint16_t section_set_add(section_set_t* set, section_builder_t* section) {
    if (set->count >= set->capacity) {
        set->capacity *= 2;
        set->sections = xrealloc(set->sections, 
                                 set->capacity * sizeof(section_builder_t));
    }
    
    uint16_t index = set->count++;
    section_builder_t* sec = &set->sections[index];
    
    memcpy(sec, section, sizeof(section_builder_t));
    
    return index;
}

// 判断段是否应该包含在静态库中
static bool should_include_section(Elf64_Shdr* shdr, const char* name) {
    // 跳过NULL段
    if (shdr->sh_type == SHT_NULL) {
        return false;
    }
    
    // 检查名称是否有效
    if (!name) {
        return false;
    }
    
    // 跳过动态链接相关段
    if (shdr->sh_type == SHT_DYNAMIC || 
        shdr->sh_type == SHT_HASH ||
        strcmp(name, ".dynsym") == 0 ||
        strcmp(name, ".dynstr") == 0 ||
        strcmp(name, ".gnu.version") == 0 ||
        strcmp(name, ".gnu.version_r") == 0 ||
        strcmp(name, ".gnu.hash") == 0) {
        return false;
    }
    
    // 跳过程序头相关段
    if (strcmp(name, ".interp") == 0) {
        return false;
    }
    
    // 包含代码和数据段
    if (shdr->sh_type == SHT_PROGBITS || 
        shdr->sh_type == SHT_NOBITS ||
        shdr->sh_type == SHT_INIT_ARRAY ||
        shdr->sh_type == SHT_FINI_ARRAY ||
        shdr->sh_type == SHT_PREINIT_ARRAY) {
        return true;
    }
    
    // 包含重定位段
    if (shdr->sh_type == SHT_RELA || shdr->sh_type == SHT_REL) {
        return true;
    }
    
    // 包含符号表和字符串表
    if (shdr->sh_type == SHT_SYMTAB || shdr->sh_type == SHT_STRTAB) {
        return true;
    }
    
    // 包含注释段
    if (shdr->sh_type == SHT_NOTE) {
        return true;
    }
    
    return false;
}

// 从动态库提取段
result_t extract_sections(void* elf_file, section_set_t** sections_out) {
    elf_file_t* elf = (elf_file_t*)elf_file;
    section_set_t* sections = section_set_create();
    
    // 遍历所有段
    for (uint16_t i = 1; i < elf->shnum; i++) {
        Elf64_Shdr* shdr = &elf->shdrs[i];
        const char* name = elf_get_section_name(elf, i);
        
        if (!should_include_section(shdr, name)) {
            continue;
        }
        
        section_builder_t sec = {0};
        sec.name = name ? xstrdup(name) : xstrdup("");
        sec.type = shdr->sh_type;
        sec.flags = shdr->sh_flags;
        sec.addr = 0;  // 可重定位文件中地址为0
        sec.align = shdr->sh_addralign;
        sec.link = shdr->sh_link;
        sec.info = shdr->sh_info;
        sec.entsize = shdr->sh_entsize;
        sec.size = shdr->sh_size;
        
        // 复制段数据
        if (shdr->sh_type != SHT_NOBITS && shdr->sh_size > 0) {
            sec.data = xmalloc(shdr->sh_size);
            memcpy(sec.data, elf->data + shdr->sh_offset, shdr->sh_size);
        }
        
        section_set_add(sections, &sec);
    }
    
    *sections_out = sections;
    return OK();
}

// 转换段属性
result_t convert_sections(section_set_t* sections) {
    if (!sections) return OK();
    
    // 对于可重定位文件，清除某些标志
    for (size_t i = 1; i < sections->count; i++) {
        section_builder_t* sec = &sections->sections[i];
        
        // 保留段的基本属性，但清除动态链接特定标志
        // 对于.text, .data, .rodata等段，保持其执行、写入、分配标志
        
        // 重置地址为0（可重定位文件）
        sec->addr = 0;
    }
    
    return OK();
}

// 释放段集合
void section_set_free(section_set_t* sections) {
    if (!sections) return;
    
    for (size_t i = 0; i < sections->count; i++) {
        section_builder_t* sec = &sections->sections[i];
        SAFE_FREE(sec->name);
        SAFE_FREE(sec->data);
    }
    
    SAFE_FREE(sections->sections);
    free(sections);
}

