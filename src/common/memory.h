#pragma once

#include "types.h"
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

// 内存池结构
// 用于减少频繁的 malloc/free 调用，提高内存分配效率
typedef struct {
    void* buffer;          // 内存池缓冲区
    size_t capacity;       // 总容量
    size_t used;           // 已使用大小
    bool owns_buffer;      // 是否拥有缓冲区所有权
} memory_pool_t;

// 内存池初始化（外部提供缓冲区）
static inline result_t memory_pool_init(memory_pool_t* pool, void* buffer, size_t size) {
    if (!pool || !buffer || size == 0) {
        return ERR(ERR_INVALID_ARGUMENT, "Invalid arguments for memory pool init");
    }
    
    pool->buffer = buffer;
    pool->capacity = size;
    pool->used = 0;
    pool->owns_buffer = false;
    return OK();
}

// 内存池创建（自动分配）
static inline result_t memory_pool_create(memory_pool_t* pool, size_t initial_size) {
    if (!pool || initial_size == 0) {
        return ERR(ERR_INVALID_ARGUMENT, "Invalid arguments for memory pool create");
    }
    
    pool->buffer = malloc(initial_size);
    if (!pool->buffer) {
        return ERR(ERR_MEMORY_ALLOC, "Failed to allocate memory pool");
    }
    
    pool->capacity = initial_size;
    pool->used = 0;
    pool->owns_buffer = true;
    return OK();
}

// 从内存池分配内存
static inline void* memory_pool_alloc(memory_pool_t* pool, size_t size) {
    if (!pool || !pool->buffer || size == 0) {
        return nullptr;
    }
    
    // 对齐到 8 字节
    size_t aligned_size = (size + 7) & ~7;
    
    // 检查空间是否足够
    if (pool->used + aligned_size > pool->capacity) {
        return nullptr;
    }
    
    void* ptr = (uint8_t*)pool->buffer + pool->used;
    pool->used += aligned_size;
    return ptr;
}

// 重置内存池（不释放）
static inline void memory_pool_reset(memory_pool_t* pool) {
    if (pool) {
        pool->used = 0;
    }
}

// 释放内存池
static inline void memory_pool_free(memory_pool_t* pool) {
    if (!pool) return;
    
    if (pool->owns_buffer && pool->buffer) {
        free(pool->buffer);
    }
    
    pool->buffer = nullptr;
    pool->capacity = 0;
    pool->used = 0;
    pool->owns_buffer = false;
}

// 获取剩余可用空间
static inline size_t memory_pool_available(const memory_pool_t* pool) {
    if (!pool) return 0;
    return pool->capacity - pool->used;
}

// 视图模式：数据切片
// 避免复制原始数据，只提供视图引用
typedef struct {
    const uint8_t* data;   // 数据指针（不拥有所有权）
    size_t size;           // 数据大小
    size_t offset;         // 当前偏移
} data_view_t;

// 创建数据视图
static inline result_t data_view_create(data_view_t* view, const void* data, size_t size) {
    if (!view || !data || size == 0) {
        return ERR(ERR_INVALID_ARGUMENT, "Invalid arguments for data view creation");
    }
    
    view->data = (const uint8_t*)data;
    view->size = size;
    view->offset = 0;
    return OK();
}

// 从视图读取指定大小的数据（返回指针，零拷贝）
static inline const void* data_view_read(data_view_t* view, size_t size) {
    if (!view || !view->data) {
        return nullptr;
    }
    
    // 检查边界
    if (view->offset + size > view->size) {
        return nullptr;
    }
    
    const void* ptr = view->data + view->offset;
    view->offset += size;
    return ptr;
}

// 获取视图剩余大小
static inline size_t data_view_remaining(const data_view_t* view) {
    if (!view) return 0;
    return view->size - view->offset;
}

// 重置视图偏移到开头
static inline void data_view_rewind(data_view_t* view) {
    if (view) {
        view->offset = 0;
    }
}

// 设置视图偏移
static inline result_t data_view_seek(data_view_t* view, size_t offset) {
    if (!view || offset > view->size) {
        return ERR_AT(ERR_INVALID_ARGUMENT, "Invalid seek offset", offset, view->size);
    }
    
    view->offset = offset;
    return OK();
}

// 创建子视图（不复制数据）
static inline result_t data_view_subview(data_view_t* parent, 
                                         data_view_t* child,
                                         size_t offset,
                                         size_t size) {
    if (!parent || !child) {
        return ERR(ERR_INVALID_ARGUMENT, "Invalid arguments for subview creation");
    }
    
    if (offset + size > parent->size) {
        return ERR_MISMATCH(ERR_ELF_INVALID_SECTION, 
                           "Subview out of bounds", 
                           offset + size, 
                           parent->size);
    }
    
    child->data = parent->data + offset;
    child->size = size;
    child->offset = 0;
    return OK();
}

// 资源管理器：统一管理多个资源的释放
#define MAX_RESOURCES 64

typedef enum {
    RESOURCE_TYPE_NONE = 0,
    RESOURCE_TYPE_MALLOC,     // malloc 分配的内存
    RESOURCE_TYPE_FILE,       // 文件描述符
    RESOURCE_TYPE_MMAP,       // mmap 映射的内存
    RESOURCE_TYPE_CUSTOM      // 自定义释放函数
} resource_type_t;

typedef void (*resource_destructor_t)(void* resource);

typedef struct {
    void* ptr;
    resource_type_t type;
    resource_destructor_t custom_destroy;
    const char* name;        // 资源名称（调试用）
} resource_entry_t;

typedef struct {
    resource_entry_t entries[MAX_RESOURCES];
    size_t count;
} resource_manager_t;

// 初始化资源管理器
static inline void resource_manager_init(resource_manager_t* mgr) {
    if (!mgr) return;
    memset(mgr->entries, 0, sizeof(mgr->entries));
    mgr->count = 0;
}

// 注册 malloc 资源
static inline result_t resource_register_malloc(resource_manager_t* mgr, 
                                                void* ptr,
                                                const char* name) {
    if (!mgr || !ptr || mgr->count >= MAX_RESOURCES) {
        return ERR(ERR_INVALID_ARGUMENT, "Cannot register resource");
    }
    
    mgr->entries[mgr->count].ptr = ptr;
    mgr->entries[mgr->count].type = RESOURCE_TYPE_MALLOC;
    mgr->entries[mgr->count].custom_destroy = nullptr;
    mgr->entries[mgr->count].name = name;
    mgr->count++;
    
    return OK();
}

// 注册自定义资源
static inline result_t resource_register_custom(resource_manager_t* mgr,
                                                void* ptr,
                                                resource_destructor_t destroy_fn,
                                                const char* name) {
    if (!mgr || !ptr || !destroy_fn || mgr->count >= MAX_RESOURCES) {
        return ERR(ERR_INVALID_ARGUMENT, "Cannot register custom resource");
    }
    
    mgr->entries[mgr->count].ptr = ptr;
    mgr->entries[mgr->count].type = RESOURCE_TYPE_CUSTOM;
    mgr->entries[mgr->count].custom_destroy = destroy_fn;
    mgr->entries[mgr->count].name = name;
    mgr->count++;
    
    return OK();
}

// 释放所有资源
static inline void resource_manager_free(resource_manager_t* mgr) {
    if (!mgr) return;
    
    for (size_t i = 0; i < mgr->count; i++) {
        resource_entry_t* entry = &mgr->entries[i];
        
        switch (entry->type) {
            case RESOURCE_TYPE_MALLOC:
                if (entry->ptr) {
                    free(entry->ptr);
                }
                break;
                
            case RESOURCE_TYPE_CUSTOM:
                if (entry->ptr && entry->custom_destroy) {
                    entry->custom_destroy(entry->ptr);
                }
                break;
                
            default:
                break;
        }
        
        entry->ptr = nullptr;
        entry->type = RESOURCE_TYPE_NONE;
    }
    
    mgr->count = 0;
}

// 取消注册资源（不释放，由调用者管理）
static inline void* resource_unregister(resource_manager_t* mgr, size_t index) {
    if (!mgr || index >= mgr->count) {
        return nullptr;
    }
    
    void* ptr = mgr->entries[index].ptr;
    
    // 向前移动后续资源
    for (size_t i = index; i < mgr->count - 1; i++) {
        mgr->entries[i] = mgr->entries[i + 1];
    }
    
    mgr->count--;
    return ptr;
}
