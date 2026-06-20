#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

// 测试断言宏
#define TEST_ASSERT(condition) \
    do { \
        if (!(condition)) { \
            fprintf(stderr, "  ❌ ASSERT FAILED: %s\n", #condition); \
            fprintf(stderr, "     at %s:%d\n", __FILE__, __LINE__); \
            return false; \
        } \
    } while(0)

#define TEST_ASSERT_EQ(expected, actual) \
    do { \
        __typeof__(expected) _exp = (expected); \
        __typeof__(actual) _act = (actual); \
        if (_exp != _act) { \
            fprintf(stderr, "  ❌ ASSERT EQ FAILED: %s == %s\n", #expected, #actual); \
            fprintf(stderr, "     Expected: %ld, Actual: %ld\n", (long)_exp, (long)_act); \
            fprintf(stderr, "     at %s:%d\n", __FILE__, __LINE__); \
            return false; \
        } \
    } while(0)

#define TEST_ASSERT_NEQ(a, b) \
    do { \
        if ((a) == (b)) { \
            fprintf(stderr, "  ❌ ASSERT NEQ FAILED: %s != %s\n", #a, #b); \
            fprintf(stderr, "     Value: %ld\n", (long)(a)); \
            fprintf(stderr, "     at %s:%d\n", __FILE__, __LINE__); \
            return false; \
        } \
    } while(0)

#define TEST_ASSERT_NOT_NULL(ptr) \
    do { \
        if ((ptr) == NULL) { \
            fprintf(stderr, "  ❌ ASSERT NOT NULL FAILED: %s\n", #ptr); \
            fprintf(stderr, "     at %s:%d\n", __FILE__, __LINE__); \
            return false; \
        } \
    } while(0)

#define TEST_ASSERT_NULL(ptr) \
    do { \
        if ((ptr) != NULL) { \
            fprintf(stderr, "  ❌ ASSERT NULL FAILED: %s\n", #ptr); \
            fprintf(stderr, "     Value: %p\n", (void*)(ptr)); \
            fprintf(stderr, "     at %s:%d\n", __FILE__, __LINE__); \
            return false; \
        } \
    } while(0)

#define TEST_ASSERT_OK(result) \
    do { \
        result_t _res = (result); \
        if (_res.code != ERR_OK) { \
            fprintf(stderr, "  ❌ ASSERT OK FAILED: %s\n", #result); \
            fprintf(stderr, "     Error code: %d, Message: %s\n", _res.code, _res.message); \
            fprintf(stderr, "     at %s:%d\n", __FILE__, __LINE__); \
            return false; \
        } \
    } while(0)

#define TEST_ASSERT_ERR(result, expected_code) \
    do { \
        result_t _res = (result); \
        if (_res.code != (expected_code)) { \
            fprintf(stderr, "  ❌ ASSERT ERR FAILED: %s\n", #result); \
            fprintf(stderr, "     Expected error: %d, Got: %d\n", (expected_code), _res.code); \
            fprintf(stderr, "     at %s:%d\n", __FILE__, __LINE__); \
            return false; \
        } \
    } while(0)

// 测试用例定义
typedef struct {
    const char* name;
    bool (*test_fn)(void);
} test_case_t;

// 测试运行器
typedef struct {
    const char* suite_name;
    test_case_t* tests;
    size_t test_count;
    size_t passed;
    size_t failed;
} test_runner_t;

// 初始化测试运行器
static inline void test_runner_init(test_runner_t* runner, 
                                    const char* name,
                                    test_case_t* tests,
                                    size_t count) {
    runner->suite_name = name;
    runner->tests = tests;
    runner->test_count = count;
    runner->passed = 0;
    runner->failed = 0;
}

// 运行单个测试
static inline bool run_test(test_case_t* test) {
    printf("  Running: %s... ", test->name);
    fflush(stdout);
    
    bool result = test->test_fn();
    
    if (result) {
        printf("✓ PASSED\n");
        return true;
    } else {
        printf("✗ FAILED\n");
        return false;
    }
}

// 运行所有测试
static inline int test_runner_run(test_runner_t* runner) {
    printf("\n========================================\n");
    printf("Test Suite: %s\n", runner->suite_name);
    printf("========================================\n\n");
    
    for (size_t i = 0; i < runner->test_count; i++) {
        if (run_test(&runner->tests[i])) {
            runner->passed++;
        } else {
            runner->failed++;
        }
    }
    
    printf("\n========================================\n");
    printf("Results: %zu passed, %zu failed out of %zu tests\n", 
           runner->passed, runner->failed, runner->test_count);
    printf("========================================\n");
    
    return runner->failed > 0 ? EXIT_FAILURE : EXIT_SUCCESS;
}

// 打印测试分隔符
#define TEST_SEPARATOR() \
    printf("\n----------------------------------------\n")

// 打印测试信息
#define TEST_INFO(fmt, ...) \
    printf("  [INFO] " fmt "\n", ##__VA_ARGS__)

// 调试模式下打印详细信息
#ifdef DEBUG
#define TEST_DEBUG(fmt, ...) \
    printf("  [DEBUG] " fmt "\n", ##__VA_ARGS__)
#else
#define TEST_DEBUG(fmt, ...) ((void)0)
#endif

// 内存泄漏检测宏（需要 valgrind 支持）
#ifdef ENABLE_MEMCHECK
#include <valgrind/memcheck.h>

#define MEMCHECK_START() \
    VALGRIND_DO_LEAK_CHECK; \
    VALGRIND_DISCARD_TRANSLATIONS

#define MEMCHECK_END() \
    VALGRIND_DO_QUICK_LEAK_CHECK

#else
#define MEMCHECK_START() ((void)0)
#define MEMCHECK_END() ((void)0)
#endif
