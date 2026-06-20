#include "common/types.h"
#include "converter/so2a.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>

// 显示使用帮助
static void print_usage(const char* prog) {
    printf("Usage: %s [OPTIONS] <input.so> <output.a>\n", prog);
    printf("\n");
    printf("Convert .so shared library to .a static library\n");
    printf("\n");
    printf("Options:\n");
    printf("  -v, --verbose      Verbose output\n");
    printf("  -k, --keep-temp    Keep temporary files\n");
    printf("  -h, --help         Show this help\n");
    printf("\n");
    printf("Example:\n");
    printf("  %s libfoo.so libfoo.a\n", prog);
}

// 显示版本信息
static void print_version(void) {
    printf("so2a 1.0.0\n");
    printf("Convert .so to .a for AArch64\n");
}

int main(int argc, char* argv[]) {
    convert_options_t options = {0};
    
    // 解析命令行参数
    static struct option long_options[] = {
        {"verbose",   no_argument, 0, 'v'},
        {"keep-temp", no_argument, 0, 'k'},
        {"help",      no_argument, 0, 'h'},
        {"version",   no_argument, 0, 'V'},
        {0, 0, 0, 0}
    };
    
    int opt;
    int option_index = 0;
    
    while ((opt = getopt_long(argc, argv, "vkhV", long_options, &option_index)) != -1) {
        switch (opt) {
            case 'v':
                options.verbose = true;
                break;
            case 'k':
                options.keep_temp = true;
                break;
            case 'h':
                print_usage(argv[0]);
                return 0;
            case 'V':
                print_version();
                return 0;
            default:
                print_usage(argv[0]);
                return 1;
        }
    }
    
    // 检查参数
    if (optind + 2 != argc) {
        fprintf(stderr, "Error: Missing input or output file\n");
        print_usage(argv[0]);
        return 1;
    }
    
    options.input_file = argv[optind];
    options.output_file = argv[optind + 1];
    
    // 执行转换
    result_t result = convert_so_to_archive(&options);
    
    if (result.code != ERR_OK) {
        return 1;
    }
    
    return 0;
}

