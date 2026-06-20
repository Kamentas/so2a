#pragma once

#include "types.h"
#include <stdio.h>
#include <string.h>

// 获取错误代码的字符串描述
static inline const char* error_to_string(error_t code) {
    switch (code) {
        case ERR_OK: return "Success";
        case ERR_ELF_BAD_MAGIC: return "Invalid ELF magic number";
        case ERR_ELF_UNSUPPORTED_CLASS: return "Unsupported ELF class (32/64-bit)";
        case ERR_ELF_UNSUPPORTED_ENDIAN: return "Unsupported ELF endianness";
        case ERR_ELF_UNSUPPORTED_VERSION: return "Unsupported ELF version";
        case ERR_ELF_INVALID_SECTION: return "Invalid section";
        case ERR_ELF_SECTION_TYPE_MISMATCH: return "Section type mismatch";
        case ERR_ELF_SECTION_ALIGN_ERROR: return "Section alignment error";
        case ERR_ELF_SYMBOL_NOT_FOUND: return "Symbol not found";
        case ERR_ELF_STRING_TABLE_ERROR: return "String table error";
        case ERR_ELF_STRING_MISSING_NUL: return "String missing NUL terminator";
        case ERR_ELF_RELOCATION_ERROR: return "Relocation error";
        case ERR_FILE_NOT_FOUND: return "File not found";
        case ERR_READ_FAILED: return "Read failed";
        case ERR_WRITE_FAILED: return "Write failed";
        case ERR_MEMORY_ALLOC: return "Memory allocation failed";
        case ERR_INTEGER_OVERFLOW: return "Integer overflow";
        case ERR_UNSUPPORTED_ARCH: return "Unsupported architecture";
        case ERR_INVALID_ARGUMENT: return "Invalid argument";
        case ERR_NOT_IMPLEMENTED: return "Not implemented";
        case ERR_UNKNOWN: return "Unknown error";
        default: return "Unknown error code";
    }
}

// 打印错误详情（用于调试）
static inline void print_error_details(const result_t* result) {
    if (!result || result->code == ERR_OK) {
        return;
    }
    
    fprintf(stderr, "Error: %s\n", result->message);
    fprintf(stderr, "Code: %d (%s)\n", result->code, error_to_string(result->code));
    
    // 根据错误类型打印额外信息
    switch (result->code) {
        case ERR_ELF_BAD_MAGIC:
            fprintf(stderr, "  Found magic: %02X %02X %02X %02X\n",
                    result->info.raw_data.data[0],
                    result->info.raw_data.data[1],
                    result->info.raw_data.data[2],
                    result->info.raw_data.data[3]);
            break;
            
        case ERR_ELF_UNSUPPORTED_CLASS:
        case ERR_ELF_SECTION_TYPE_MISMATCH:
            fprintf(stderr, "  Found: %u, Expected: %u\n",
                    result->info.mismatch.found,
                    result->info.mismatch.expected);
            break;
            
        case ERR_ELF_STRING_MISSING_NUL:
        case ERR_ELF_INVALID_SECTION:
            fprintf(stderr, "  Offset: 0x%llx, Size: %llu\n",
                    (unsigned long long)result->info.location.offset,
                    (unsigned long long)result->info.location.size);
            break;
            
        default:
            break;
    }
}

// 检查结果并打印错误（调试模式）
#ifdef DEBUG
#define CHECK_RESULT(result) do { \
    if ((result).code != ERR_OK) { \
        fprintf(stderr, "\n[DEBUG] Error at %s:%d\n", __FILE__, __LINE__); \
        print_error_details(&(result)); \
        fprintf(stderr, "\n"); \
    } \
} while(0)
#else
#define CHECK_RESULT(result) ((void)0)
#endif

// 验证条件，如果失败返回错误
#define VERIFY(condition, error_code, error_msg) do { \
    if (!(condition)) { \
        return ERR(error_code, error_msg); \
    } \
} while(0)

// 验证条件，如果失败返回带上下文的错误
#define VERIFY_AT(condition, error_code, error_msg, off, sz) do { \
    if (!(condition)) { \
        return ERR_AT(error_code, error_msg, off, sz); \
    } \
} while(0)
