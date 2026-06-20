#pragma once

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

// 日志级别枚举
typedef enum {
    LOG_LEVEL_DEBUG = 0,
    LOG_LEVEL_INFO = 1,
    LOG_LEVEL_WARN = 2,
    LOG_LEVEL_ERROR = 3,
    LOG_LEVEL_NONE = 4
} log_level_t;

// 全局日志配置
typedef struct {
    log_level_t level;       // 当前日志级别
    FILE* output;            // 输出流（stderr 或文件）
    bool use_color;          // 是否使用颜色输出
    bool show_timestamp;     // 是否显示时间戳
    bool show_location;      // 是否显示文件行号
} log_config_t;

// 全局日志实例
static log_config_t g_log_config = {
    .level = LOG_LEVEL_INFO,
    .output = NULL,  // 运行时设置为 stderr
    .use_color = true,
    .show_timestamp = false,
    .show_location = true
};

// ANSI 颜色代码
#define COLOR_RESET   "\033[0m"
#define COLOR_RED     "\033[31m"
#define COLOR_GREEN   "\033[32m"
#define COLOR_YELLOW  "\033[33m"
#define COLOR_BLUE    "\033[34m"
#define COLOR_MAGENTA "\033[35m"
#define COLOR_CYAN    "\033[36m"

// 获取日志级别字符串
static inline const char* log_level_to_string(log_level_t level) {
    switch (level) {
        case LOG_LEVEL_DEBUG: return "DEBUG";
        case LOG_LEVEL_INFO:  return "INFO ";
        case LOG_LEVEL_WARN:  return "WARN ";
        case LOG_LEVEL_ERROR: return "ERROR";
        case LOG_LEVEL_NONE:  return "NONE ";
        default:              return "UNKNO";
    }
}

// 获取日志级别颜色
static inline const char* log_level_to_color(log_level_t level) {
    switch (level) {
        case LOG_LEVEL_DEBUG: return COLOR_CYAN;
        case LOG_LEVEL_INFO:  return COLOR_GREEN;
        case LOG_LEVEL_WARN:  return COLOR_YELLOW;
        case LOG_LEVEL_ERROR: return COLOR_RED;
        case LOG_LEVEL_NONE:  return COLOR_RESET;
        default:              return COLOR_RESET;
    }
}

// 设置日志级别
static inline void log_set_level(log_level_t level) {
    g_log_config.level = level;
}

// 设置日志输出流
static inline void log_set_output(FILE* output) {
    if (output) {
        g_log_config.output = output;
    }
}

// 启用/禁用颜色
static inline void log_set_color(bool enabled) {
    g_log_config.use_color = enabled;
}

// 初始化日志系统
static inline void log_init(void) {
    // 默认输出到 stderr
    if (!g_log_config.output) {
        g_log_config.output = stderr;
    }
    
    // 检查是否需要禁用颜色（例如输出到文件）
    if (g_log_config.output != stderr && g_log_config.output != stdout) {
        g_log_config.use_color = false;
    }
}

// 内部日志宏实现
#define _LOG_INTERNAL(level, fmt, ...) \
    do { \
        if (g_log_config.level <= level) { \
            if (g_log_config.use_color) { \
                fprintf(g_log_config.output, "%s", log_level_to_color(level)); \
            } \
            fprintf(g_log_config.output, "[%s]", log_level_to_string(level)); \
            if (g_log_config.show_location) { \
                fprintf(g_log_config.output, " [%s:%d]", __FILE__, __LINE__); \
            } \
            fprintf(g_log_config.output, " " fmt "\n", ##__VA_ARGS__); \
            if (g_log_config.use_color) { \
                fprintf(g_log_config.output, "%s", COLOR_RESET); \
            } \
            fflush(g_log_config.output); \
        } \
    } while(0)

// 调试日志
#ifdef DEBUG
#define LOG_DEBUG(fmt, ...) _LOG_INTERNAL(LOG_LEVEL_DEBUG, fmt, ##__VA_ARGS__)
#else
#define LOG_DEBUG(fmt, ...) ((void)0)
#endif

// 信息日志
#define LOG_INFO(fmt, ...) _LOG_INTERNAL(LOG_LEVEL_INFO, fmt, ##__VA_ARGS__)

// 警告日志
#define LOG_WARN(fmt, ...) _LOG_INTERNAL(LOG_LEVEL_WARN, fmt, ##__VA_ARGS__)

// 错误日志
#define LOG_ERROR(fmt, ...) _LOG_INTERNAL(LOG_LEVEL_ERROR, fmt, ##__VA_ARGS__)

// 打印十六进制数据
static inline void log_hex_dump(const char* label, 
                                const uint8_t* data, 
                                size_t len,
                                size_t offset) {
    if (g_log_config.level > LOG_LEVEL_DEBUG) {
        return;
    }
    
    const size_t bytes_per_line = 16;
    
    if (label) {
        LOG_DEBUG("%s (offset=0x%zx, size=%zu):", label, offset, len);
    }
    
    for (size_t i = 0; i < len; i += bytes_per_line) {
        // 打印偏移
        fprintf(g_log_config.output, "  0x%04zx: ", i + offset);
        
        // 打印十六进制数据
        for (size_t j = 0; j < bytes_per_line; j++) {
            if (i + j < len) {
                fprintf(g_log_config.output, "%02x ", data[i + j]);
            } else {
                fprintf(g_log_config.output, "   ");
            }
            
            // 每 8 字节添加分隔
            if (j == 7) {
                fprintf(g_log_config.output, " ");
            }
        }
        
        // 打印 ASCII 表示
        fprintf(g_log_config.output, " |");
        for (size_t j = 0; j < bytes_per_line && (i + j) < len; j++) {
            uint8_t c = data[i + j];
            if (c >= 32 && c < 127) {
                fprintf(g_log_config.output, "%c", c);
            } else {
                fprintf(g_log_config.output, ".");
            }
        }
        fprintf(g_log_config.output, "|\n");
    }
    
    fflush(g_log_config.output);
}

// 简化版十六进制打印
#define LOG_HEX(label, value, size) \
    log_hex_dump(label, (const uint8_t*)&value, size, 0)

// ELF 结构体打印辅助函数
#ifdef DEBUG

// 打印 ELF 文件头信息
#include "../elf/aarch64/parser.h"

static inline void log_elf_header(const Elf64_Ehdr* ehdr) {
    if (!ehdr || g_log_config.level > LOG_LEVEL_DEBUG) {
        return;
    }
    
    LOG_DEBUG("ELF Header @ %p:", ehdr);
    LOG_DEBUG("  Magic:           %02X %02X %02X %02X", 
              ehdr->e_ident[0], ehdr->e_ident[1], 
              ehdr->e_ident[2], ehdr->e_ident[3]);
    LOG_DEBUG("  Class:           %u", ehdr->e_ident[4]);
    LOG_DEBUG("  Data:            %u", ehdr->e_ident[5]);
    LOG_DEBUG("  Type:            %u", ehdr->e_type);
    LOG_DEBUG("  Machine:         %u", ehdr->e_machine);
    LOG_DEBUG("  Entry:           0x%lx", ehdr->e_entry);
    LOG_DEBUG("  PH Offset:       0x%lx", ehdr->e_phoff);
    LOG_DEBUG("  SH Offset:       0x%lx", ehdr->e_shoff);
    LOG_DEBUG("  Sections:        %u", ehdr->e_shnum);
    LOG_DEBUG("  Section Size:    %u", ehdr->e_shentsize);
}

// 打印 ELF 段头信息
static inline void log_section_header(const Elf64_Shdr* shdr, size_t index) {
    if (!shdr || g_log_config.level > LOG_LEVEL_DEBUG) {
        return;
    }
    
    LOG_DEBUG("Section %zu @ %p:", index, shdr);
    LOG_DEBUG("  Name Offset:     %u", shdr->sh_name);
    LOG_DEBUG("  Type:            %u", shdr->sh_type);
    LOG_DEBUG("  Flags:           0x%lx", shdr->sh_flags);
    LOG_DEBUG("  Address:         0x%lx", shdr->sh_addr);
    LOG_DEBUG("  Offset:          0x%lx", shdr->sh_offset);
    LOG_DEBUG("  Size:            %lu", shdr->sh_size);
    LOG_DEBUG("  Link:            %u", shdr->sh_link);
    LOG_DEBUG("  Info:            %u", shdr->sh_info);
    LOG_DEBUG("  Alignment:       %lu", shdr->sh_addralign);
    LOG_DEBUG("  Entry Size:      %lu", shdr->sh_entsize);
}

#else

#define log_elf_header(ehdr) ((void)0)
#define log_section_header(shdr, idx) ((void)0)

#endif

// 性能计时器（仅 DEBUG 模式）
#ifdef DEBUG
#include <time.h>

typedef struct {
    clock_t start;
    clock_t end;
    double elapsed_ms;
} timer_t;

static inline void timer_start(timer_t* timer) {
    if (!timer) return;
    timer->start = clock();
}

static inline void timer_stop(timer_t* timer) {
    if (!timer) return;
    timer->end = clock();
    timer->elapsed_ms = (double)(timer->end - timer->start) * 1000.0 / CLOCKS_PER_SEC;
}

static inline void timer_log_result(timer_t* timer, const char* operation) {
    if (!timer || !operation) return;
    LOG_DEBUG("[PERF] %s took %.3f ms", operation, timer->elapsed_ms);
}

#define TIMER_START(name) \
    timer_t name; \
    timer_start(&name)

#define TIMER_STOP_AND_LOG(name, op) \
    timer_stop(&name); \
    timer_log_result(&name, op)

#else

#define TIMER_START(name) ((void)0)
#define TIMER_STOP_AND_LOG(name, op) ((void)0)

#endif
