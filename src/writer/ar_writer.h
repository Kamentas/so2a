#pragma once

#include "../common/types.h"

// AR归档格式魔数
#define AR_MAGIC "!<arch>\n"
#define AR_MAGIC_SIZE 8

// AR文件头（60字节）
typedef struct {
    char ar_name[16];     // 文件名
    char ar_date[12];     // 修改时间（十进制秒）
    char ar_uid[6];       // 用户ID
    char ar_gid[6];       // 组ID
    char ar_mode[8];      // 文件模式（八进制）
    char ar_size[10];     // 文件大小（十进制）
    char ar_fmag[2];      // 文件魔数（"`\n"）
} ar_header_t;

#define AR_HEADER_SIZE 60
#define AR_FMAG "`\n"

// AR写入器
typedef struct {
    const char* output_path;  // 输出路径
    int fd;                   // 文件描述符
    size_t offset;            // 当前偏移
} ar_writer_t;

// 创建AR写入器
ar_writer_t* ar_writer_create(const char* output_path);

// 添加文件到归档
result_t ar_writer_add_file(ar_writer_t* writer, const char* filename, 
                             const uint8_t* data, size_t size);

// 完成写入
result_t ar_writer_finalize(ar_writer_t* writer);

// 释放写入器
void ar_writer_free(ar_writer_t* writer);

// 便捷函数：创建.a文件
result_t create_archive(const char* output_path, const char* object_file);

