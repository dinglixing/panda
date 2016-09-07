#include "array.h"
#include "interp.h"

intptr_t array_create(env_t *env, int ac, val_t *av)
{
    array_t *array;
    int size = ac < DEF_ELEM_SIZE ? DEF_ELEM_SIZE : ac;

    if (ac > UINT16_MAX) {
        env_set_error(env, ERR_ResourceOutLimit);
        return 0;
    }

    array = env_heap_alloc(env, sizeof(array_t) + sizeof(val_t) * size);
    if (array) {
        val_t *vals = (val_t *)(array + 1);

        array->magic = MAGIC_ARRAY;
        array->age = 0;
        array->elem_size = size;
        array->elem_bgn  = 0;
        array->elem_end  = ac;
        array->elems = vals;

        memcpy(vals, av, sizeof(val_t) * ac);
    }

    return (intptr_t) array;
}

void array_elem_get(env_t *env, val_t *a, val_t *i, val_t *elem)
{
    array_t *array = (array_t *) val_2_intptr(a);
    int id = val_2_integer(i);

    if (id >= 0 && id < array_length(array)) {
        *elem = array->elems[array->elem_bgn + id];
    } else {
        val_set_undefined(elem);
    }
}

void array_elem_set(env_t *env, val_t *a, val_t *i, val_t *elem)
{
    array_t *array = (array_t *) val_2_intptr(a);
    int id = val_2_integer(i);

    if (id >= 0 && id < array_length(array)) {
        array->elems[array->elem_bgn + id] = *elem;
    } else {
        val_set_undefined(elem);
    }
}

static array_t *array_space_extend_tail(env_t *env, val_t *self, int n)
{
    array_t *a = (array_t *)val_2_intptr(self);
    val_t *elems;
    int len;

    if (a->elem_size - a->elem_end > n) {
        return a;
    }
    len = array_length(a);

    if (a->elem_size -len > n) {
        memmove(a->elems, a->elems + a->elem_bgn, len);
        a->elem_bgn = 0;
        a->elem_end = len;
        return a;
    } else {
        int size = SIZE_ALIGN_16(len + n);
        if (size > UINT16_MAX) {
            env_set_error(env, ERR_ResourceOutLimit);
            return NULL;
        }
        elems = env_heap_alloc(env, size * sizeof(val_t));
        if (elems) {
            a = (array_t *)val_2_intptr(self);
            memcpy(elems, a->elems + a->elem_bgn, sizeof(val_t) * len);
            a->elem_size = size;
            a->elem_bgn = 0;
            a->elem_end = len;
            return a;
        } else {
            return NULL;
        }
    }
}

static array_t *array_space_extend_head(env_t *env, val_t *self, int n)
{
    array_t *a = (array_t *)val_2_intptr(self);
    val_t *elems;
    int len;

    if (a->elem_bgn > n) {
        return a;
    }
    len = array_length(a);

    if (a->elem_size - len > n) {
        n = a->elem_size - a->elem_end;
        memmove(a->elems + a->elem_bgn + n, a->elems + a->elem_bgn, len);
        a->elem_bgn += n;
        a->elem_end += n;
        return a;
    } else {
        int size = SIZE_ALIGN_16(len + n);
        if (size > UINT16_MAX) {
            env_set_error(env, ERR_ResourceOutLimit);
            return NULL;
        }
        elems = env_heap_alloc(env, size * sizeof(val_t));
        if (elems) {
            a = (array_t *)val_2_intptr(self);
            memcpy(elems + size - len, a->elems + a->elem_bgn, sizeof(val_t) * len);
            a->elem_size = size;
            a->elem_bgn = size - len;
            a->elem_end = size;
            return a;
        } else {
            return NULL;
        }
    }
}

val_t array_push(env_t *env, int ac, val_t *av)
{
    if (ac > 1 && val_is_array(av)) {
        int n = ac - 1;
        array_t *a = array_space_extend_tail(env, av, n);

        if (a) {
            memcpy(a->elems + a->elem_end, av + 1, sizeof(val_t) * n);
            a->elem_end += n;
            return val_mk_number(array_length(a));
        }
    } else {
        env_set_error(env, ERR_InvalidInput);
    }
    return val_mk_undefined();
}

val_t array_unshift(env_t *env, int ac, val_t *av)
{
    if (ac > 1 && val_is_array(av)) {
        int n = ac - 1;
        array_t *a = array_space_extend_head(env, av, n);

        if (a) {
            memcpy(a->elems + a->elem_bgn - n, av + 1, sizeof(val_t) * n);
            a->elem_bgn -= n;
            return val_mk_number(array_length(a));
        }
    } else {
        env_set_error(env, ERR_InvalidInput);
    }
    return val_mk_undefined();
}

val_t array_pop(env_t *env, int ac, val_t *av)
{
    if (ac > 0 && val_is_array(av)) {
        array_t *a = (array_t *)val_2_intptr(av);

        if (array_length(a)) {
            return a->elems[--a->elem_end];
        }
    } else {
        env_set_error(env, ERR_InvalidInput);
    }
    return val_mk_undefined();
}

val_t array_shift(env_t *env, int ac, val_t *av)
{
    if (ac > 0 && val_is_array(av)) {
        array_t *a = (array_t *)val_2_intptr(av);

        if (array_length(a)) {
            return a->elems[a->elem_bgn++];
        }
    } else {
        env_set_error(env, ERR_InvalidInput);
    }
    return val_mk_undefined();
}

val_t array_foreach(env_t *env, int ac, val_t *av)
{
    if (ac > 1 && val_is_array(av) && val_is_function(av + 1)) {
        array_t *a = (array_t *)val_2_intptr(av);
        int i, max = array_length(a);

        for (i = 0; i < max && !env->error; i++) {
            val_t key = val_mk_number(i);

            env_push_call_argument(env, &key);
            env_push_call_argument(env, array_values(a) + i);
            env_push_call_function(env, av + 1);

            interp_execute_call(env, 2);
        }
    }

    return val_mk_undefined();
}
