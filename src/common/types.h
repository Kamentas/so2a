#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

// ELF基础类型
typedef uint64_t Elf64_Addr;
typedef uint64_t Elf64_Off;
typedef uint16_t Elf64_Half;
typedef uint32_t Elf64_Word;
typedef int32_t  Elf64_Sword;
typedef uint64_t Elf64_Xword;
typedef int64_t  Elf64_Sxword;

// 错误码
typedef enum {
    ERR_OK = 0,
    ERR_FILE_NOT_FOUND,
    ERR_INVALID_ELF,
    ERR_UNSUPPORTED_ARCH,
    ERR_MEMORY_ALLOC,
    ERR_INVALID_SECTION,
    ERR_WRITE_FAILED,
    ERR_READ_FAILED
} error_t;

// 返回值结构
typedef struct {
    error_t code;
    const char* message;
} result_t;

#define OK() ((result_t){ERR_OK, nullptr})
#define ERR(code, msg) ((result_t){code, msg})

// 内存管理辅助
#define SAFE_FREE(ptr) do { if (ptr) { free(ptr); ptr = nullptr; } } while(0)

// C23特性：typeof 和 auto
#define AUTO_CAST(type, value) ((type)(value))

