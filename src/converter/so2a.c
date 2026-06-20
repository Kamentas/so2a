#include "so2a.h"
#include "../elf/aarch64/parser.h"
#include "../converter/section_builder.h"
#include "../converter/symbol_table.h"
#include "../converter/relocator.h"
#include "../writer/elf_writer.h"
#include "../writer/ar_writer.h"
#include "../common/utils.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>

// 生成临时文件名
static char* generate_temp_filename(const char* prefix) {
    static char temp[256];
    snprintf(temp, sizeof(temp), "%s.tmp.%d.o", prefix, getpid());
    return temp;
}

// 转换.so到.a
result_t convert_so_to_archive(const convert_options_t* options) {
    result_t result;
    elf_file_t* elf = nullptr;
    section_set_t* sections = nullptr;
    symbol_table_t* symtab = nullptr;
    reloc_set_t* relocs = nullptr;
    elf_writer_t* writer = nullptr;
    char* temp_obj = nullptr;
    
    // 解析输入ELF文件
    if (options->verbose) {
        printf("Parsing: %s\n", options->input_file);
    }
    
    result = elf_parse(options->input_file, &elf);
    if (result.code != ERR_OK) {
        fprintf(stderr, "Error: %s\n", result.message);
        goto cleanup;
    }
    
    // 提取段
    if (options->verbose) {
        printf("Extracting sections\n");
    }
    
    result = extract_sections(elf, &sections);
    if (result.code != ERR_OK) {
        fprintf(stderr, "Error: %s\n", result.message);
        goto cleanup;
    }
    
    if (options->verbose) {
        printf("  %zu sections extracted\n", sections->count);
    }
    
    // 转换段属性
    result = convert_sections(sections);
    if (result.code != ERR_OK) {
        fprintf(stderr, "Error: %s\n", result.message);
        goto cleanup;
    }
    
    // 提取符号
    if (options->verbose) {
        printf("Extracting symbols\n");
    }
    
    result = extract_symbols(elf, &symtab);
    if (result.code != ERR_OK) {
        fprintf(stderr, "Error: %s\n", result.message);
        goto cleanup;
    }
    
    if (options->verbose) {
        printf("  %zu symbols extracted\n", symtab->count);
    }
    
    // 转换符号
    result = convert_dynamic_symbols(symtab);
    if (result.code != ERR_OK) {
        fprintf(stderr, "Error: %s\n", result.message);
        goto cleanup;
    }
    
    // 提取重定位
    if (options->verbose) {
        printf("Extracting relocations\n");
    }
    
    result = extract_relocations(elf, &relocs);
    if (result.code != ERR_OK) {
        fprintf(stderr, "Error: %s\n", result.message);
        goto cleanup;
    }
    
    if (options->verbose) {
        printf("  %zu relocation tables extracted\n", relocs->count);
    }
    
    // 转换重定位
    if (options->verbose) {
        printf("Converting relocations\n");
        fflush(stdout);
    }
    
    result = convert_dynamic_relocs(relocs);
    if (result.code != ERR_OK) {
        fprintf(stderr, "Error: %s\n", result.message);
        goto cleanup;
    }
    
    if (options->verbose) {
        printf("  Relocations converted\n");
        fflush(stdout);
    }
    
    // 创建临时对象文件
    temp_obj = generate_temp_filename(options->output_file);
    
    if (options->verbose) {
        printf("Writing object file: %s\n", temp_obj);
        fflush(stdout);
    }
    
    // 创建ELF写入器
    if (options->verbose) {
        printf("Creating ELF writer\n");
        fflush(stdout);
    }
    
    writer = elf_writer_create(sections, symtab, relocs);
    if (!writer) {
        result = ERR(ERR_MEMORY_ALLOC, "Cannot create ELF writer");
        fprintf(stderr, "Error creating writer\n");
        goto cleanup;
    }
    
    if (options->verbose) {
        printf("ELF writer created\n");
        fflush(stdout);
    }
    
    // 写入对象文件
    result = elf_writer_write(writer, temp_obj);
    if (result.code != ERR_OK) {
        fprintf(stderr, "Error: %s\n", result.message);
        goto cleanup;
    }
    
    // 创建归档文件
    if (options->verbose) {
        printf("Creating archive: %s\n", options->output_file);
    }
    
    result = create_archive(options->output_file, temp_obj);
    if (result.code != ERR_OK) {
        fprintf(stderr, "Error: %s\n", result.message);
        goto cleanup;
    }
    
    if (options->verbose) {
        printf("Conversion completed successfully\n");
    } else {
        printf("Created: %s\n", options->output_file);
    }
    
    result = OK();
    
cleanup:
    // 清理临时文件
    if (temp_obj && !options->keep_temp) {
        unlink(temp_obj);
    }
    
    // 释放资源
    elf_writer_free(writer);
    reloc_set_free(relocs);
    symbol_table_free(symtab);
    section_set_free(sections);
    elf_free(elf);
    
    return result;
}

