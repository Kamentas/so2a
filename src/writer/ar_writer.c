#include "ar_writer.h"
#include "../common/utils.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdarg.h>

// 创建AR写入器
ar_writer_t* ar_writer_create(const char* output_path) {
    ar_writer_t* writer = xcalloc(1, sizeof(ar_writer_t));
    writer->output_path = output_path;
    
    // 创建文件
    writer->fd = open(output_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (writer->fd < 0) {
        free(writer);
        return nullptr;
    }
    
    // 写入AR魔数
    if (write(writer->fd, AR_MAGIC, AR_MAGIC_SIZE) != AR_MAGIC_SIZE) {
        close(writer->fd);
        free(writer);
        return nullptr;
    }
    
    writer->offset = AR_MAGIC_SIZE;
    
    return writer;
}

// 格式化AR头字段（右对齐，空格填充）
static void format_ar_field(char* dest, size_t size, const char* fmt, ...) {
    char buf[32];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    
    size_t len = strlen(buf);
    memset(dest, ' ', size);
    memcpy(dest, buf, len < size ? len : size);
}

// 添加文件到归档
result_t ar_writer_add_file(ar_writer_t* writer, const char* filename,
                             const uint8_t* data, size_t size) {
    if (!writer || !filename || !data) {
        return ERR(ERR_WRITE_FAILED, "Invalid parameters");
    }
    
    // 创建AR头
    ar_header_t header;
    memset(&header, ' ', sizeof(header));
    
    // 文件名（截断到15字符，保留'/'）
    size_t name_len = strlen(filename);
    if (name_len > 15) {
        name_len = 15;
    }
    memcpy(header.ar_name, filename, name_len);
    header.ar_name[name_len] = '/';
    
    // 时间戳
    time_t now = time(nullptr);
    format_ar_field(header.ar_date, 12, "%lu", (unsigned long)now);
    
    // UID/GID（使用0）
    format_ar_field(header.ar_uid, 6, "0");
    format_ar_field(header.ar_gid, 6, "0");
    
    // 权限（0644）
    format_ar_field(header.ar_mode, 8, "100644");
    
    // 文件大小
    format_ar_field(header.ar_size, 10, "%zu", size);
    
    // 魔数
    memcpy(header.ar_fmag, AR_FMAG, 2);
    
    // 写入头
    if (write(writer->fd, &header, AR_HEADER_SIZE) != AR_HEADER_SIZE) {
        return ERR(ERR_WRITE_FAILED, "Failed to write AR header");
    }
    
    // 写入数据
    if (write(writer->fd, data, size) != (ssize_t)size) {
        return ERR(ERR_WRITE_FAILED, "Failed to write file data");
    }
    
    writer->offset += AR_HEADER_SIZE + size;
    
    // AR格式要求每个文件对齐到2字节
    if (size % 2 != 0) {
        char padding = '\n';
        if (write(writer->fd, &padding, 1) != 1) {
            return ERR(ERR_WRITE_FAILED, "Failed to write padding");
        }
        writer->offset++;
    }
    
    return OK();
}

// 完成写入
result_t ar_writer_finalize(ar_writer_t* writer) {
    if (!writer) {
        return ERR(ERR_WRITE_FAILED, "Invalid writer");
    }
    
    // AR归档不需要特殊的结束标记
    // 只需确保所有数据已写入
    fsync(writer->fd);
    
    return OK();
}

// 释放写入器
void ar_writer_free(ar_writer_t* writer) {
    if (!writer) return;
    
    if (writer->fd >= 0) {
        close(writer->fd);
    }
    
    free(writer);
}

// 便捷函数：创建.a文件
result_t create_archive(const char* output_path, const char* object_file) {
    // 读取对象文件
    int fd = open(object_file, O_RDONLY);
    if (fd < 0) {
        return ERR(ERR_FILE_NOT_FOUND, "Cannot open object file");
    }
    
    struct stat st;
    if (fstat(fd, &st) < 0) {
        close(fd);
        return ERR(ERR_READ_FAILED, "Cannot stat object file");
    }
    
    size_t size = st.st_size;
    uint8_t* data = xmalloc(size);
    
    if (read(fd, data, size) != (ssize_t)size) {
        close(fd);
        free(data);
        return ERR(ERR_READ_FAILED, "Cannot read object file");
    }
    
    close(fd);
    
    // 创建归档
    ar_writer_t* writer = ar_writer_create(output_path);
    if (!writer) {
        free(data);
        return ERR(ERR_WRITE_FAILED, "Cannot create archive");
    }
    
    // 提取文件名（不包含路径）
    const char* basename = strrchr(object_file, '/');
    if (basename) {
        basename++;
    } else {
        basename = object_file;
    }
    
    result_t result = ar_writer_add_file(writer, basename, data, size);
    free(data);
    
    if (result.code != ERR_OK) {
        ar_writer_free(writer);
        return result;
    }
    
    result = ar_writer_finalize(writer);
    ar_writer_free(writer);
    
    return result;
}

