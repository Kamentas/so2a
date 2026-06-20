#pragma once

// so2a 通用工具头文件
// 整合所有优化的公共模块

#include "types.h"        // 错误处理和类型定义
#include "error.h"        // 错误处理辅助函数
#include "endian.h"       // 端序转换抽象层
#include "memory.h"       // 内存池和资源管理
#include "logging.h"      // 日志和调试工具
#include "string_table.h" // 零拷贝字符串表
#include "utils.h"        // 内存分配辅助函数
// #include "elf_common.h"   // 通用 ELF 定义 - 与架构特定定义冲突，不直接包含

// 库版本信息
#define SO2A_VERSION_MAJOR 1
#define SO2A_VERSION_MINOR 0
#define SO2A_VERSION_PATCH 0
#define SO2A_VERSION "1.0.0"

// 库特性标志
#define SO2A_FEATURE_ZERO_COPY      1  // 支持零拷贝设计
#define SO2A_FEATURE_ENDIAN_AWARE   1  // 支持端序感知
#define SO2A_FEATURE_MEMORY_POOL    1  // 支持内存池
#define SO2A_FEATURE_DETAILED_ERROR 1  // 支持详细错误信息
#define SO2A_FEATURE_LOGGING        1  // 支持日志系统
#define SO2A_FEATURE_TESTING        1  // 支持测试框架

// 初始化宏（必须在使用库之前调用）
static inline void so2a_init(void) {
    // 初始化日志系统
    log_init();
    
    // 初始化本机端序（如果需要）
    init_native_endian();
    
    LOG_DEBUG("so2a library initialized (version %s)", SO2A_VERSION);
}

// 清理宏（在程序退出时调用）
static inline void so2a_cleanup(void) {
    LOG_DEBUG("so2a library cleanup");
}

// 常用宏定义

// 未使用参数标记
#define UNUSED(x) (void)(x)

// 数组元素数量
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

// 最小值/最大值
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

// 对齐向上取整
#define ALIGN_UP(val, align) (((val) + (align) - 1) & ~((align) - 1))

// 对齐向下取整
#define ALIGN_DOWN(val, align) ((val) & ~((align) - 1))

// 检查是否对齐
#define IS_ALIGNED(val, align) (((val) & ((align) - 1)) == 0)

// 位操作
#define BIT(n) (1UL << (n))
#define SET_BIT(reg, bit) ((reg) |= BIT(bit))
#define CLEAR_BIT(reg, bit) ((reg) &= ~BIT(bit))
#define TEST_BIT(reg, bit) (((reg) & BIT(bit)) != 0)

// 字节序转换（便捷宏）
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define HTONS(x) __builtin_bswap16(x)
#define HTONL(x) __builtin_bswap32(x)
#define HTONLL(x) __builtin_bswap64(x)
#else
#define HTONS(x) (x)
#define HTONL(x) (x)
#define HTONLL(x) (x)
#endif

// 编译时检查
#define STATIC_ASSERT(condition, name) \
    typedef char name[(condition) ? 1 : -1]

// 结构体成员偏移
#ifndef offsetof
#define offsetof(type, member) __builtin_offsetof(type, member)
#endif

// 容器宏（从成员指针获取结构体指针）
#ifndef container_of
#define container_of(ptr, type, member) \
    ((type*)((char*)(ptr) - offsetof(type, member)))
#endif

// 可能不返回的函数标记
#define NORETURN __attribute__((noreturn))

// 纯函数标记（无副作用）
#define PURE __attribute__((pure))

// 常量函数标记（只读参数）
#define CONST __attribute__((const))

// 可能为空的指针标记
#define NULLABLE

// 不允许为空的指针标记
#define NONNULL __attribute__((nonnull))

// 格式字符串检查
#define PRINTF(fmt, args) __attribute__((format(printf, fmt, args)))

// 内联提示
#define ALWAYS_INLINE inline __attribute__((always_inline))
#define NEVER_INLINE __attribute__((noinline))

// 分支预测提示
#define LIKELY(x) __builtin_expect(!!(x), 1)
#define UNLIKELY(x) __builtin_expect(!!(x), 0)

// 缓存行大小（用于避免伪共享）
#define CACHE_LINE_SIZE 64

// 对齐到缓存行
#define CACHE_ALIGNED __attribute__((aligned(CACHE_LINE_SIZE)))

// 废弃的函数标记
#define DEPRECATED __attribute__((deprecated))

// 替代的函数标记
#define DEPRECATED_REPLACE(new_func) \
    __attribute__((deprecated("Use " #new_func " instead")))

// 调试断言
#ifdef DEBUG
#define DEBUG_ASSERT(expr) \
    do { \
        if (!(expr)) { \
            fprintf(stderr, "Assertion failed: %s\n", #expr); \
            fprintf(stderr, "  at %s:%d\n", __FILE__, __LINE__); \
            abort(); \
        } \
    } while(0)
#else
#define DEBUG_ASSERT(expr) ((void)0)
#endif

// 运行时检查（release 模式也保留）
#define RUNTIME_CHECK(expr, error_code, error_msg) \
    do { \
        if (!(expr)) { \
            return ERR(error_code, error_msg); \
        } \
    } while(0)

// 空值检查
#define CHECK_NULL(ptr) \
    do { \
        if (!(ptr)) { \
            return ERR(ERR_INVALID_ARGUMENT, "Null pointer: " #ptr); \
        } \
    } while(0)

// 结果传播宏（类似 Rust 的 ? 运算符）
#define TRY(expr) \
    do { \
        result_t _result = (expr); \
        if (_result.code != ERR_OK) { \
            return _result; \
        } \
    } while(0)

// 条件执行宏
#define WHEN_DEBUG(expr) \
    do { \
        ifdef DEBUG \
        expr; \
        endif \
    } while(0)
