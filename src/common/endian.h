#pragma once

#include "../common/types.h"
#include <stdint.h>
#include <stdbool.h>

// 端序类型枚举
typedef enum {
    ENDIAN_LITTLE = 0,     // 小端序
    ENDIAN_BIG = 1,        // 大端序
    ENDIAN_NATIVE = 2      // 本机端序（编译时确定）
} endian_type_t;

// 端序转换器接口
// 参考 rust-elf-master 的 EndianParse trait 设计
typedef struct {
    endian_type_t type;
    
    // 解析 u16
    uint16_t (*parse_u16)(const uint8_t* data, size_t* offset);
    
    // 解析 u32
    uint32_t (*parse_u32)(const uint8_t* data, size_t* offset);
    
    // 解析 u64
    uint64_t (*parse_u64)(const uint8_t* data, size_t* offset);
    
    // 解析 i32
    int32_t (*parse_i32)(const uint8_t* data, size_t* offset);
    
    // 解析 i64
    int64_t (*parse_i64)(const uint8_t* data, size_t* offset);
    
    // 判断是否为小端序
    bool (*is_little_endian)(void);
} endian_converter_t;

// 小端序转换实现
static inline uint16_t parse_u16_le(const uint8_t* data, size_t* offset) {
    uint16_t value = (uint16_t)data[*offset] | 
                     (uint16_t)data[*offset + 1] << 8;
    *offset += 2;
    return value;
}

static inline uint32_t parse_u32_le(const uint8_t* data, size_t* offset) {
    uint32_t value = (uint32_t)data[*offset] | 
                     (uint32_t)data[*offset + 1] << 8 |
                     (uint32_t)data[*offset + 2] << 16 |
                     (uint32_t)data[*offset + 3] << 24;
    *offset += 4;
    return value;
}

static inline uint64_t parse_u64_le(const uint8_t* data, size_t* offset) {
    uint64_t value = (uint64_t)data[*offset] | 
                     (uint64_t)data[*offset + 1] << 8 |
                     (uint64_t)data[*offset + 2] << 16 |
                     (uint64_t)data[*offset + 3] << 24 |
                     (uint64_t)data[*offset + 4] << 32 |
                     (uint64_t)data[*offset + 5] << 40 |
                     (uint64_t)data[*offset + 6] << 48 |
                     (uint64_t)data[*offset + 7] << 56;
    *offset += 8;
    return value;
}

static inline int32_t parse_i32_le(const uint8_t* data, size_t* offset) {
    uint32_t uval = parse_u32_le(data, offset);
    return *(int32_t*)&uval;
}

static inline int64_t parse_i64_le(const uint8_t* data, size_t* offset) {
    uint64_t uval = parse_u64_le(data, offset);
    return *(int64_t*)&uval;
}

// 大端序转换实现
static inline uint16_t parse_u16_be(const uint8_t* data, size_t* offset) {
    uint16_t value = (uint16_t)data[*offset + 1] | 
                     (uint16_t)data[*offset] << 8;
    *offset += 2;
    return value;
}

static inline uint32_t parse_u32_be(const uint8_t* data, size_t* offset) {
    uint32_t value = (uint32_t)data[*offset + 3] | 
                     (uint32_t)data[*offset + 2] << 8 |
                     (uint32_t)data[*offset + 1] << 16 |
                     (uint32_t)data[*offset] << 24;
    *offset += 4;
    return value;
}

static inline uint64_t parse_u64_be(const uint8_t* data, size_t* offset) {
    uint64_t value = (uint64_t)data[*offset + 7] | 
                     (uint64_t)data[*offset + 6] << 8 |
                     (uint64_t)data[*offset + 5] << 16 |
                     (uint64_t)data[*offset + 4] << 24 |
                     (uint64_t)data[*offset + 3] << 32 |
                     (uint64_t)data[*offset + 2] << 40 |
                     (uint64_t)data[*offset + 1] << 48 |
                     (uint64_t)data[*offset] << 56;
    *offset += 8;
    return value;
}

static inline int32_t parse_i32_be(const uint8_t* data, size_t* offset) {
    uint32_t uval = parse_u32_be(data, offset);
    return *(int32_t*)&uval;
}

static inline int64_t parse_i64_be(const uint8_t* data, size_t* offset) {
    uint64_t uval = parse_u64_be(data, offset);
    return *(int64_t*)&uval;
}

// 本机端序检测
static inline bool is_native_little_endian(void) {
    uint16_t test = 1;
    return *((uint8_t*)&test) == 1;
}

// 辅助函数：返回真
static inline bool func_true(void) {
    return true;
}

// 辅助函数：返回假
static inline bool func_false(void) {
    return false;
}

// 小端序转换器实例
static const endian_converter_t endian_little = {
    .type = ENDIAN_LITTLE,
    .parse_u16 = parse_u16_le,
    .parse_u32 = parse_u32_le,
    .parse_u64 = parse_u64_le,
    .parse_i32 = parse_i32_le,
    .parse_i64 = parse_i64_le,
    .is_little_endian = func_true
};

// 大端序转换器实例
static const endian_converter_t endian_big = {
    .type = ENDIAN_BIG,
    .parse_u16 = parse_u16_be,
    .parse_u32 = parse_u32_be,
    .parse_u64 = parse_u64_be,
    .parse_i32 = parse_i32_be,
    .parse_i64 = parse_i64_be,
    .is_little_endian = func_false
};

// 本机端序转换器实例（运行时确定）
static endian_converter_t endian_native_runtime = {
    .type = ENDIAN_NATIVE,
    .is_little_endian = is_native_little_endian
};

// 初始化本机端序转换器
static inline void init_native_endian(void) {
    if (is_native_little_endian()) {
        endian_native_runtime.parse_u16 = parse_u16_le;
        endian_native_runtime.parse_u32 = parse_u32_le;
        endian_native_runtime.parse_u64 = parse_u64_le;
        endian_native_runtime.parse_i32 = parse_i32_le;
        endian_native_runtime.parse_i64 = parse_i64_le;
    } else {
        endian_native_runtime.parse_u16 = parse_u16_be;
        endian_native_runtime.parse_u32 = parse_u32_be;
        endian_native_runtime.parse_u64 = parse_u64_be;
        endian_native_runtime.parse_i32 = parse_i32_be;
        endian_native_runtime.parse_i64 = parse_i64_be;
    }
}

// 根据 ELF 文件的 EI_DATA 字段获取端序转换器
// EI_DATA: ELFDATA2LSB(1) 或 ELFDATA2MSB(2)
static inline const endian_converter_t* get_endian_converter(uint8_t ei_data) {
    switch (ei_data) {
        case 1: // ELFDATA2LSB - 小端序
            return &endian_little;
        case 2: // ELFDATA2MSB - 大端序
            return &endian_big;
        default:
            return nullptr;
    }
}

// 动态端序转换器（运行时根据文件端序选择）
typedef struct {
    const endian_converter_t* converter;
    bool is_initialized;
} dynamic_endian_t;

// 初始化动态端序转换器
static inline result_t init_dynamic_endian(dynamic_endian_t* dyn, uint8_t ei_data) {
    if (!dyn) {
        return ERR(ERR_INVALID_ARGUMENT, "Invalid argument: dyn is NULL");
    }
    
    const endian_converter_t* conv = get_endian_converter(ei_data);
    if (!conv) {
        return ERR_MISMATCH(ERR_ELF_UNSUPPORTED_ENDIAN, 
                           "Unsupported ELF endianness", 
                           ei_data, 
                           1);
    }
    
    dyn->converter = conv;
    dyn->is_initialized = true;
    return OK();
}

// 使用动态端序转换器解析 u16
static inline uint16_t dynamic_parse_u16(dynamic_endian_t* dyn, 
                                         const uint8_t* data, 
                                         size_t* offset) {
    if (!dyn->is_initialized || !dyn->converter) {
        return 0;
    }
    return dyn->converter->parse_u16(data, offset);
}

// 使用动态端序转换器解析 u32
static inline uint32_t dynamic_parse_u32(dynamic_endian_t* dyn, 
                                         const uint8_t* data, 
                                         size_t* offset) {
    if (!dyn->is_initialized || !dyn->converter) {
        return 0;
    }
    return dyn->converter->parse_u32(data, offset);
}

// 使用动态端序转换器解析 u64
static inline uint64_t dynamic_parse_u64(dynamic_endian_t* dyn, 
                                         const uint8_t* data, 
                                         size_t* offset) {
    if (!dyn->is_initialized || !dyn->converter) {
        return 0;
    }
    return dyn->converter->parse_u64(data, offset);
}

// 检查是否是小端序
static inline bool check_is_little_endian(const endian_converter_t* conv) {
    if (!conv) return false;
    return conv->is_little_endian();
}
