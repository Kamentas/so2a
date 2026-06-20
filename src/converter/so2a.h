#pragma once

#include "../common/types.h"

// 转换选项
typedef struct {
    const char* input_file;   // 输入.so文件
    const char* output_file;  // 输出.a文件
    bool verbose;             // 详细输出
    bool keep_temp;           // 保留临时文件
} convert_options_t;

// 转换.so到.a
result_t convert_so_to_archive(const convert_options_t* options);

