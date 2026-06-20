#include "elf_writer.h"
#include "../common/utils.h"
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

// 创建ELF写入器
elf_writer_t* elf_writer_create(section_set_t* sections,
                                 symbol_table_t* symtab,
                                 reloc_set_t* relocs) {
    elf_writer_t* writer = xcalloc(1, sizeof(elf_writer_t));
    writer->sections = sections;
    writer->symtab = symtab;
    writer->relocs = relocs;
    
    // 创建段名字符串表
    writer->shstrtab = string_table_create();
    
    // 创建符号字符串表
    writer->strtab = string_table_create();
    
    return writer;
}

// 写入缓冲区
static void write_buffer(elf_writer_t* writer, const void* data, size_t size) {
    size_t required = writer->buffer_offset + size;
    
    if (required > writer->buffer_size) {
        size_t new_size = writer->buffer_size == 0 ? 65536 : writer->buffer_size;
        while (new_size < required) {
            new_size *= 2;
        }
        writer->buffer = xrealloc(writer->buffer, new_size);
        writer->buffer_size = new_size;
    }
    
    memcpy(writer->buffer + writer->buffer_offset, data, size);
    writer->buffer_offset += size;
}

// 写入填充
static void write_padding(elf_writer_t* writer, size_t align) {
    size_t offset = writer->buffer_offset;
    size_t aligned = align_up(offset, align);
    size_t padding = aligned - offset;
    
    if (padding > 0) {
        uint8_t zeros[64] = {0};
        while (padding > 0) {
            size_t chunk = padding > 64 ? 64 : padding;
            write_buffer(writer, zeros, chunk);
            padding -= chunk;
        }
    }
}

// 构建段头字符串表
static void build_shstrtab(elf_writer_t* writer) {
    for (size_t i = 0; i < writer->sections->count; i++) {
        section_builder_t* sec = &writer->sections->sections[i];
        if (sec->name && sec->name[0] != '\0') {
            string_table_add(writer->shstrtab, sec->name);
        }
    }
    
    // 添加符号表相关段名
    string_table_add(writer->shstrtab, ".symtab");
    string_table_add(writer->shstrtab, ".strtab");
    string_table_add(writer->shstrtab, ".shstrtab");
}

// 构建符号字符串表
static void build_strtab(elf_writer_t* writer) {
    for (size_t i = 0; i < writer->symtab->count; i++) {
        symbol_entry_t* sym = &writer->symtab->entries[i];
        if (sym->name && sym->name[0] != '\0') {
            string_table_add(writer->strtab, sym->name);
        }
    }
}

// 获取段名在字符串表中的偏移
static uint32_t get_section_name_offset(elf_writer_t* writer, const char* name) {
    if (!name || name[0] == '\0') return 0;
    
    uint32_t offset = 0;
    for (size_t i = 1; i < writer->shstrtab->size; ) {
        const char* str = &writer->shstrtab->data[i];
        if (strcmp(str, name) == 0) {
            return i;
        }
        size_t len = strlen(str);
        if (len == 0) break;  // 防止无限循环
        i += len + 1;
    }
    return offset;
}

// 获取符号名在字符串表中的偏移
static uint32_t get_string_offset(elf_writer_t* writer, const char* name) {
    if (!name || name[0] == '\0') return 0;
    
    for (size_t i = 1; i < writer->strtab->size; ) {
        const char* str = &writer->strtab->data[i];
        if (strcmp(str, name) == 0) {
            return i;
        }
        size_t len = strlen(str);
        if (len == 0) break;  // 防止无限循环
        i += len + 1;
    }
    return 0;
}

// 写入ELF文件
result_t elf_writer_write(elf_writer_t* writer, const char* output_path) {
    // 构建字符串表
    build_shstrtab(writer);
    build_strtab(writer);
    
    // 计算段数量（包括添加的符号表相关段）
    uint16_t shnum = writer->sections->count + 3;  // +symtab +strtab +shstrtab
    
    // 写入ELF头
    Elf64_Ehdr ehdr = {0};
    memcpy(ehdr.e_ident, ELF_MAGIC, 4);
    ehdr.e_ident[4] = ELFCLASS64;
    ehdr.e_ident[5] = ELFDATA2LSB;
    ehdr.e_ident[6] = EV_CURRENT;
    ehdr.e_ident[7] = 0;  // ELFOSABI_NONE
    
    ehdr.e_type = ET_REL;  // 可重定位文件
    ehdr.e_machine = EM_AARCH64;
    ehdr.e_version = EV_CURRENT;
    ehdr.e_entry = 0;
    ehdr.e_phoff = 0;  // 没有程序头
    ehdr.e_shoff = sizeof(Elf64_Ehdr);  // 段头紧跟ELF头
    ehdr.e_flags = 0;
    ehdr.e_ehsize = sizeof(Elf64_Ehdr);
    ehdr.e_phentsize = 0;
    ehdr.e_phnum = 0;
    ehdr.e_shentsize = sizeof(Elf64_Shdr);
    ehdr.e_shnum = shnum;
    ehdr.e_shstrndx = shnum - 1;  // 最后一个段是.shstrtab
    
    write_buffer(writer, &ehdr, sizeof(ehdr));
    
    // 预留段头表空间
    size_t shdr_offset = writer->buffer_offset;
    writer->buffer_offset += shnum * sizeof(Elf64_Shdr);
    
    // 写入各个段数据并记录偏移
    Elf64_Shdr* shdrs = xcalloc(shnum, sizeof(Elf64_Shdr));
    
    // 第一个段是NULL段
    // 已经初始化为0
    
    // 写入原有段
    for (size_t i = 1; i < writer->sections->count; i++) {
        section_builder_t* sec = &writer->sections->sections[i];
        Elf64_Shdr* shdr = &shdrs[i];
        
        // 对齐
        write_padding(writer, sec->align > 0 ? sec->align : 1);
        
        shdr->sh_name = get_section_name_offset(writer, sec->name);
        shdr->sh_type = sec->type;
        shdr->sh_flags = sec->flags;
        shdr->sh_addr = sec->addr;
        shdr->sh_offset = writer->buffer_offset;
        shdr->sh_size = sec->size;
        shdr->sh_link = sec->link;
        shdr->sh_info = sec->info;
        shdr->sh_addralign = sec->align > 0 ? sec->align : 1;
        shdr->sh_entsize = sec->entsize;
        
        // 写入段数据
        if (sec->data && sec->size > 0) {
            write_buffer(writer, sec->data, sec->size);
        }
    }
    
    // 写入符号表
    uint16_t symtab_idx = writer->sections->count;
    uint16_t strtab_idx = symtab_idx + 1;
    uint16_t shstrtab_idx = strtab_idx + 1;
    
    write_padding(writer, 8);
    Elf64_Shdr* symtab_shdr = &shdrs[symtab_idx];
    symtab_shdr->sh_name = get_section_name_offset(writer, ".symtab");
    symtab_shdr->sh_type = SHT_SYMTAB;
    symtab_shdr->sh_flags = 0;
    symtab_shdr->sh_addr = 0;
    symtab_shdr->sh_offset = writer->buffer_offset;
    symtab_shdr->sh_size = writer->symtab->count * sizeof(Elf64_Sym);
    symtab_shdr->sh_link = strtab_idx;  // 链接到字符串表
    symtab_shdr->sh_info = 1;  // 第一个非局部符号的索引
    symtab_shdr->sh_addralign = 8;
    symtab_shdr->sh_entsize = sizeof(Elf64_Sym);
    
    // 转换并写入符号
    for (size_t i = 0; i < writer->symtab->count; i++) {
        symbol_entry_t* sym_entry = &writer->symtab->entries[i];
        Elf64_Sym sym = {0};
        
        sym.st_name = get_string_offset(writer, sym_entry->name);
        sym.st_info = ELF64_ST_INFO(sym_entry->bind, sym_entry->type);
        sym.st_other = ELF64_ST_VISIBILITY(sym_entry->visibility);
        sym.st_shndx = sym_entry->section_idx;
        sym.st_value = sym_entry->value;
        sym.st_size = sym_entry->size;
        
        write_buffer(writer, &sym, sizeof(sym));
    }
    
    // 写入符号字符串表
    write_padding(writer, 1);
    Elf64_Shdr* strtab_shdr = &shdrs[strtab_idx];
    strtab_shdr->sh_name = get_section_name_offset(writer, ".strtab");
    strtab_shdr->sh_type = SHT_STRTAB;
    strtab_shdr->sh_flags = 0;
    strtab_shdr->sh_addr = 0;
    strtab_shdr->sh_offset = writer->buffer_offset;
    strtab_shdr->sh_size = writer->strtab->size;
    strtab_shdr->sh_link = 0;
    strtab_shdr->sh_info = 0;
    strtab_shdr->sh_addralign = 1;
    strtab_shdr->sh_entsize = 0;
    
    write_buffer(writer, writer->strtab->data, writer->strtab->size);
    
    // 写入段名字符串表
    write_padding(writer, 1);
    Elf64_Shdr* shstrtab_shdr = &shdrs[shstrtab_idx];
    shstrtab_shdr->sh_name = get_section_name_offset(writer, ".shstrtab");
    shstrtab_shdr->sh_type = SHT_STRTAB;
    shstrtab_shdr->sh_flags = 0;
    shstrtab_shdr->sh_addr = 0;
    shstrtab_shdr->sh_offset = writer->buffer_offset;
    shstrtab_shdr->sh_size = writer->shstrtab->size;
    shstrtab_shdr->sh_link = 0;
    shstrtab_shdr->sh_info = 0;
    shstrtab_shdr->sh_addralign = 1;
    shstrtab_shdr->sh_entsize = 0;
    
    write_buffer(writer, writer->shstrtab->data, writer->shstrtab->size);
    
    // 回写段头表
    memcpy(writer->buffer + shdr_offset, shdrs, shnum * sizeof(Elf64_Shdr));
    free(shdrs);
    
    // 写入文件
    int fd = open(output_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        return ERR(ERR_WRITE_FAILED, "Cannot create output file");
    }
    
    ssize_t written = write(fd, writer->buffer, writer->buffer_offset);
    close(fd);
    
    if (written != (ssize_t)writer->buffer_offset) {
        return ERR(ERR_WRITE_FAILED, "Write failed");
    }
    
    return OK();
}

// 释放写入器
void elf_writer_free(elf_writer_t* writer) {
    if (!writer) return;
    
    string_table_free(writer->shstrtab);
    string_table_free(writer->strtab);
    SAFE_FREE(writer->buffer);
    free(writer);
}

