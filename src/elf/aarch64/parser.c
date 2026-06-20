#include "parser.h"
#include "../../common/utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

// 验证ELF文件
bool elf_is_valid(const uint8_t* data, size_t size) {
    if (size < sizeof(Elf64_Ehdr)) {
        return false;
    }
    
    const Elf64_Ehdr* ehdr = (const Elf64_Ehdr*)data;
    
    // 检查魔数
    if (memcmp(ehdr->e_ident, ELF_MAGIC, 4) != 0) {
        return false;
    }
    
    // 检查类别（必须是64位）
    if (ehdr->e_ident[4] != ELFCLASS64) {
        return false;
    }
    
    // 检查字节序（小端）
    if (ehdr->e_ident[5] != ELFDATA2LSB) {
        return false;
    }
    
    return true;
}

bool elf_is_dynamic_lib(elf_file_t* elf) {
    return elf->ehdr->e_type == ET_DYN;
}

bool elf_is_aarch64(elf_file_t* elf) {
    return elf->ehdr->e_machine == EM_AARCH64;
}

// 获取段名
const char* elf_get_section_name(elf_file_t* elf, uint16_t index) {
    if (index >= elf->shnum) {
        return nullptr;
    }
    
    Elf64_Shdr* shdr = &elf->shdrs[index];
    if (shdr->sh_name >= elf->shstrtab_size) {
        return nullptr;
    }
    
    return &elf->shstrtab[shdr->sh_name];
}

// 获取字符串
const char* elf_get_string(elf_file_t* elf, uint32_t index, bool is_dynamic) {
    if (is_dynamic) {
        if (index >= elf->dynstr_size) {
            return nullptr;
        }
        return &elf->dynstr[index];
    } else {
        if (index >= elf->strtab_size) {
            return nullptr;
        }
        return &elf->strtab[index];
    }
}

// 获取符号名
const char* elf_get_symbol_name(elf_file_t* elf, Elf64_Sym* sym, bool is_dynamic) {
    const char* name = elf_get_string(elf, sym->st_name, is_dynamic);
    return name ? name : "";
}

// 查找段
section_info_t* elf_find_section(elf_file_t* elf, const char* name) {
    for (uint16_t i = 0; i < elf->shnum; i++) {
        const char* sec_name = elf_get_section_name(elf, i);
        if (sec_name && strcmp(sec_name, name) == 0) {
            section_info_t* info = xmalloc(sizeof(section_info_t));
            info->shdr = &elf->shdrs[i];
            info->name = xstrdup(sec_name);
            info->data = elf->data + info->shdr->sh_offset;
            info->size = info->shdr->sh_size;
            info->index = i;
            return info;
        }
    }
    return nullptr;
}

// 获取段
section_info_t* elf_get_section(elf_file_t* elf, uint16_t index) {
    if (index >= elf->shnum) {
        return nullptr;
    }
    
    section_info_t* info = xmalloc(sizeof(section_info_t));
    info->shdr = &elf->shdrs[index];
    info->name = xstrdup(elf_get_section_name(elf, index));
    info->data = elf->data + info->shdr->sh_offset;
    info->size = info->shdr->sh_size;
    info->index = index;
    return info;
}

// 解析ELF文件
result_t elf_parse(const char* filename, elf_file_t** elf_out) {
    // 打开文件
    int fd = open(filename, O_RDONLY);
    if (fd < 0) {
        return ERR(ERR_FILE_NOT_FOUND, "Cannot open file");
    }
    
    // 获取文件大小
    struct stat st;
    if (fstat(fd, &st) < 0) {
        close(fd);
        return ERR(ERR_READ_FAILED, "Cannot stat file");
    }
    
    size_t size = st.st_size;
    
    // 映射文件到内存
    uint8_t* data = mmap(nullptr, size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (data == MAP_FAILED) {
        close(fd);
        return ERR(ERR_READ_FAILED, "Cannot mmap file");
    }
    
    close(fd);
    
    // 验证ELF
    if (!elf_is_valid(data, size)) {
        munmap(data, size);
        return ERR(ERR_INVALID_ELF, "Invalid ELF file");
    }
    
    // 创建ELF结构
    elf_file_t* elf = xcalloc(1, sizeof(elf_file_t));
    elf->data = data;
    elf->size = size;
    elf->ehdr = (Elf64_Ehdr*)data;
    
    // 检查架构
    if (!elf_is_aarch64(elf)) {
        munmap(data, size);
        free(elf);
        return ERR(ERR_UNSUPPORTED_ARCH, "Not AArch64 architecture");
    }
    
    // 检查是否为动态库
    if (!elf_is_dynamic_lib(elf)) {
        munmap(data, size);
        free(elf);
        return ERR(ERR_INVALID_ELF, "Not a dynamic library");
    }
    
    // 解析段头表
    elf->shnum = elf->ehdr->e_shnum;
    elf->shdrs = (Elf64_Shdr*)(data + elf->ehdr->e_shoff);
    
    // 解析程序头表
    elf->phnum = elf->ehdr->e_phnum;
    if (elf->phnum > 0) {
        elf->phdrs = (Elf64_Phdr*)(data + elf->ehdr->e_phoff);
    }
    
    // 解析段名字符串表
    if (elf->ehdr->e_shstrndx < elf->shnum) {
        Elf64_Shdr* shstrtab_hdr = &elf->shdrs[elf->ehdr->e_shstrndx];
        elf->shstrtab = (char*)(data + shstrtab_hdr->sh_offset);
        elf->shstrtab_size = shstrtab_hdr->sh_size;
    }
    
    // 查找符号表和字符串表
    for (uint16_t i = 0; i < elf->shnum; i++) {
        Elf64_Shdr* shdr = &elf->shdrs[i];
        
        if (shdr->sh_type == SHT_SYMTAB) {
            elf->symtab = (Elf64_Sym*)(data + shdr->sh_offset);
            elf->symtab_num = shdr->sh_size / sizeof(Elf64_Sym);
            
            // 获取对应的字符串表
            if (shdr->sh_link < elf->shnum) {
                Elf64_Shdr* strtab_hdr = &elf->shdrs[shdr->sh_link];
                elf->strtab = (char*)(data + strtab_hdr->sh_offset);
                elf->strtab_size = strtab_hdr->sh_size;
            }
        }
        else if (shdr->sh_type == SHT_DYNSYM) {
            elf->dynsym = (Elf64_Sym*)(data + shdr->sh_offset);
            elf->dynsym_num = shdr->sh_size / sizeof(Elf64_Sym);
            
            // 获取对应的字符串表
            if (shdr->sh_link < elf->shnum) {
                Elf64_Shdr* dynstr_hdr = &elf->shdrs[shdr->sh_link];
                elf->dynstr = (char*)(data + dynstr_hdr->sh_offset);
                elf->dynstr_size = dynstr_hdr->sh_size;
            }
        }
        else if (shdr->sh_type == SHT_DYNAMIC) {
            elf->dynamic = (Elf64_Dyn*)(data + shdr->sh_offset);
            elf->dynamic_num = shdr->sh_size / sizeof(Elf64_Dyn);
        }
    }
    
    *elf_out = elf;
    return OK();
}

// 释放ELF结构
void elf_free(elf_file_t* elf) {
    if (!elf) return;
    
    if (elf->data) {
        munmap(elf->data, elf->size);
    }
    
    free(elf);
}

