#include "panda.h"

static int set_output_name(const char *input, void *buf, int sz)
{
    int pos = file_base_name(input, buf, sz - 5);

    if (pos < 1) {
        return -1;
    }

    memcpy(buf + pos, ".pdc", 5);

    return 0;
}

#define OUTPUT_NAME_MAX     (128)

int panda_compile(const char *input, void *mem_ptr, int mem_size)
{
    char *output  = (char *) mem_ptr;
    void *cpl_mem, *exe_mem;
    env_t env;
    int cpl_mem_sz, exe_mem_sz, input_sz;
    int exe_sz;

    exe_mem_sz = (mem_size / 3) & (~0xf);
    cpl_mem_sz = mem_size - OUTPUT_NAME_MAX - exe_mem_sz;

    exe_mem = mem_ptr + OUTPUT_NAME_MAX;
    cpl_mem = mem_ptr + OUTPUT_NAME_MAX + exe_mem_sz;

    if (0 != set_output_name(input, output, OUTPUT_NAME_MAX)) {
        return -1;
    }

    if (0 != compile_env_init(&env, cpl_mem, cpl_mem_sz)) {
        return -1;
    }

    panda_native_init(&env);

    input = file_load(input, &input_sz);
    if (!input) {
        return -1;
    }

    exe_sz = compile_exe(&env, input, exe_mem, exe_mem_sz);
    file_release((void *)input, input_sz);

    if (exe_sz <= 0) {
        return -1;
    } else {
        return file_store(output, exe_mem, exe_sz);
    }
}

