#pragma once

#include "types.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <endian.h>

// 内存操作辅助函数
static inline void* xmalloc(size_t size) {
    void* ptr = malloc(size);
    if (!ptr) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(ERR_MEMORY_ALLOC);
    }
    return ptr;
}

static inline void* xcalloc(size_t nmemb, size_t size) {
    void* ptr = calloc(nmemb, size);
    if (!ptr) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(ERR_MEMORY_ALLOC);
    }
    return ptr;
}

static inline void* xrealloc(void* ptr, size_t size) {
    void* new_ptr = realloc(ptr, size);
    if (!new_ptr && size > 0) {
        fprintf(stderr, "Memory reallocation failed\n");
        exit(ERR_MEMORY_ALLOC);
    }
    return new_ptr;
}

// 文件操作
static inline size_t get_file_size(FILE* fp) {
    fseek(fp, 0, SEEK_END);
    size_t size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    return size;
}

// 对齐计算
static inline size_t align_up(size_t val, size_t align) {
    return (val + align - 1) & ~(align - 1);
}

// 字符串操作
static inline char* xstrdup(const char* s) {
    if (!s) return nullptr;
    char* dup = strdup(s);
    if (!dup) {
        fprintf(stderr, "String duplication failed\n");
        exit(ERR_MEMORY_ALLOC);
    }
    return dup;
}

