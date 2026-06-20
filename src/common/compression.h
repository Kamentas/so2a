#pragma once

#include "types.h"
#include <stdint.h>
#include <stddef.h>

// 压缩类型
typedef enum {
    COMPRESS_NONE = 0,
    COMPRESS_ZLIB = 1,  // ELFCOMPRESS_ZLIB
    COMPRESS_ZSTD = 2   // ELFCOMPRESS_ZSTD
} compress_type_t;

// 压缩段头（Elf64_Chdr）
typedef struct {
    uint32_t ch_type;       // 压缩类型
    uint32_t ch_reserved;   // 保留
    uint64_t ch_size;       // 解压后大小
    uint64_t ch_addralign;  // 对齐
} compress_header_t;

// 检测段是否被压缩
bool is_section_compressed(const void* data, size_t size);

// 获取压缩类型
compress_type_t get_compression_type(const void* data, size_t size);

// 解压段数据
result_t decompress_section(const uint8_t* compressed_data, size_t compressed_size,
                            uint8_t** decompressed_data, size_t* decompressed_size);

// 压缩段数据
result_t compress_section(const uint8_t* data, size_t size, compress_type_t type,
                          uint8_t** compressed_data, size_t* compressed_size);

