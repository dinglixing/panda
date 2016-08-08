


#ifndef __LANG_ENV_INC__
#define __LANG_ENV_INC__

#include "config.h"

#include "val.h"
#include "heap.h"
#include "symtbl.h"
#include "executable.h"

typedef struct scope_t {
    uint8_t type;
    uint8_t size;
    uint8_t num;
    val_t   *var_buf;
    struct scope_t *super;
} scope_t;

typedef struct env_t {
    int error;

    int fp;
    int ss;
    int sp;

    val_t *sb;

    heap_t *heap;

    heap_t heap_top;
    heap_t heap_bot;

    scope_t *scope;
    val_t *result;

    intptr_t sym_tbl;

    executable_t exe;
    /*
    // executable info
    char    *symbal_buf;
    uint32_t symbal_buf_size;

    uint32_t func_code_max;
    uint32_t func_code_end;
    uint16_t main_code_max;
    uint16_t main_code_end;

    uint16_t number_max;
    uint16_t number_num;

    uint16_t string_max;
    uint16_t string_num;

    uint16_t native_max;
    uint16_t native_num;

    uint16_t func_max;
    uint16_t func_num;

    double   *number_map;
    intptr_t *string_map;
    intptr_t *native_map;
    intptr_t *native_fns;
    uint8_t **func_map;
    uint8_t  *main_code;
    uint8_t  *func_code;
    */
} env_t;

int env_init(env_t *env, void * mem_ptr, int mem_size,
             void *heap_ptr, int heap_size, val_t *stack_ptr, int stack_size,
             int number_max, int string_max, int native_max, int func_max,
             int main_code_max, int func_code_max);

int env_deinit(env_t *env);

static inline void env_set_error(env_t *env, int error) {
    if (env) env->error = error;
}

static inline heap_t *env_heap_get_free(env_t *env) {
    return env->heap == &env->heap_top ? &env->heap_bot : &env->heap_top;
}

void *env_heap_alloc(env_t *env, int size);
void env_heap_gc(env_t *env, int level);

int env_scope_create(env_t *env, scope_t *super, int vc, int ac, val_t *av);
int env_scope_extend(env_t *env, val_t *v);
int env_scope_extend_to(env_t *env, int size);
int env_scope_get(env_t *env, int id, val_t **v);
int env_scope_set(env_t *env, int id, val_t *v);

intptr_t env_symbal_add(env_t *env, const char *name);
intptr_t env_symbal_get(env_t *env, const char *name);

int env_string_find_add(env_t *env, intptr_t s);
int env_number_find_add(env_t *env, double);

int env_native_add(env_t *env, const char *name, val_t (*fn)(env_t *, int ac, val_t *av));
int env_native_find(env_t *env, intptr_t sym_id);

int  env_frame_setup(env_t *env, uint8_t *pc, scope_t *super, int vc, int ac, val_t *av);
void env_frame_restore(env_t *env, uint8_t **pc, scope_t **scope);

#endif /* __LANG_ENV_INC__ */
