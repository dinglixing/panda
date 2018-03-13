/*
MIT License

Copyright (c) 2016 Lixing Ding <ding.lixing@gmail.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "err.h"
#include "type_string.h"

int string_compare(val_t *a, val_t *b)
{
    const char *s1 = val_2_cstring(a);
    const char *s2 = val_2_cstring(b);

    if (s1 && s2) {
        return strcmp(s1, s2);
    } else {
        return 1;
    }
}

void string_at(env_t *env, val_t *a, val_t *b, val_t *res)
{
    const char *s = val_2_cstring(a);
    int l = strlen(s);
    int i = val_2_integer(b);

    (void) env;
    if (i >= 0 && i < l) {
        val_set_inner_string(res, s[i]);
    } else {
        val_set_undefined(res);
    }
}

static inline string_t *string_alloc(env_t *env, int size)
{
    string_t *s = env_heap_alloc(env, SIZE_ALIGN(sizeof(string_t) + size));

    if (s) {
        s->magic = MAGIC_STRING;
        s->age = 0;
        s->size = size;
    }
    return s;
}

val_t string_create_heap_val(env_t *env, int size)
{
    string_t *s;

    size += 1;
    s = string_alloc(env, size);
    if (s) {
        memset(s->str, 0, size);
        return val_mk_heap_string((intptr_t)s);
    }
    return VAL_UNDEFINED;
}

void string_add(env_t *env, val_t *a, val_t *b, val_t *res)
{
    if (!val_is_string(b)) {
        val_set_nan(res);
        return;
    }

    int size1 = string_len(a);
    int size2 = string_len(b);
    int len = size1 + size2;
    string_t *s = string_alloc(env, len + 1);

    if (s) {
        // Todo: length overflow should be check! or variable length field.
        memcpy(s->str + 0, val_2_cstring(a), size1);
        memcpy(s->str + size1, val_2_cstring(b), size2 + 1);
        val_set_heap_string(res, (intptr_t) s);
    } else {
        env_set_error(env, ERR_NotEnoughMemory);
        val_set_undefined(res);
    }
}

void string_elem_get(val_t *self, int i, val_t *elem)
{
    const char *s = val_2_cstring(self);
    int len = string_len(self);

    if (i >= 0 && i < len) {
        val_set_inner_string(elem, s[i]);
    } else {
        val_set_undefined(elem);
    }
}

val_t string_length(env_t *env, int ac, val_t *av)
{
    (void) env;

    if (ac > 0) {
        return val_mk_number(string_len(av));
    } else {
        return val_mk_undefined();
    }
}

val_t string_index_of(env_t *env, int ac, val_t *av)
{
    const char *s, *f;

    if (ac < 2 || NULL == (s = val_2_cstring(av))) {
        env_set_error(env, ERR_InvalidInput);
        return val_mk_undefined();
    } else
    if (NULL == (f = val_2_cstring(av+1))) {
        return val_mk_number(-1);
    } else {
        char *pos = strstr(s, f);
        if (pos)  {
            return val_mk_number(pos - s);
        } else {
            return val_mk_number(-1);
        }
    }
}



static int string_inline_is_true(val_t *self) {
    return ((const char *)self)[4];
}

static int string_inline_compare(val_t *self, val_t *to) {
    const char *a = ((const char *)self) + 4;
    const char *b = val_2_cstring(to);

    return b ? !strcmp(a, b) : 0;
}

static int string_inline_is_gt(val_t *self, val_t *to) {
    const char *a = ((const char *)self) + 4;
    const char *b = val_2_cstring(to);

    return b ? (strcmp(a, b) > 0) : 0;
}

static int string_inline_is_ge(val_t *self, val_t *to) {
    const char *a = ((const char *)self) + 4;
    const char *b = val_2_cstring(to);

    return b ? (strcmp(a, b) >= 0) : 0;
}

static int string_inline_is_lt(val_t *self, val_t *to) {
    const char *a = ((const char *)self) + 4;
    const char *b = val_2_cstring(to);

    return b ? (strcmp(a, b) < 0) : 0;
}

static int string_inline_is_le(val_t *self, val_t *to) {
    const char *a = ((const char *)self) + 4;
    const char *b = val_2_cstring(to);

    return b ? (strcmp(a, b) <= 0) : 0;
}

static int string_heap_is_true(val_t *self) {
    return ((const char *) val_2_intptr(self))[4];
}

static int string_heap_compare(val_t *self, val_t *to) {
    const char *a = (const char *) (val_2_intptr(self) + 4);
    const char *b = val_2_cstring(to);

    return b ? !strcmp(a, b) : 0;
}

static int string_heap_is_gt(val_t *self, val_t *to) {
    const char *a = (const char *) (val_2_intptr(self) + 4);
    const char *b = val_2_cstring(to);

    return b ? (strcmp(a, b) > 0) : 0;
}

static int string_heap_is_ge(val_t *self, val_t *to) {
    const char *a = (const char *) (val_2_intptr(self) + 4);
    const char *b = val_2_cstring(to);

    return b ? (strcmp(a, b) >= 0) : 0;
}

static int string_heap_is_lt(val_t *self, val_t *to) {
    const char *a = (const char *) (val_2_intptr(self) + 4);
    const char *b = val_2_cstring(to);

    return b ? (strcmp(a, b) < 0) : 0;
}

static int string_heap_is_le(val_t *self, val_t *to) {
    const char *a = (const char *) (val_2_intptr(self) + 4);
    const char *b = val_2_cstring(to);

    return b ? (strcmp(a, b) <= 0) : 0;
}

static int string_foreign_is_true(val_t *self) {
    return ((const char *) val_2_intptr(self))[0];
}

static int string_foreign_compare(val_t *self, val_t *to) {
    const char *a = (const char *) val_2_intptr(self);
    const char *b = val_2_cstring(to);

    return b ? !strcmp(a, b) : 0;
}

static int string_foreign_is_gt(val_t *self, val_t *to) {
    const char *a = (const char *) val_2_intptr(self);
    const char *b = val_2_cstring(to);

    return b ? (strcmp(a, b) > 0) : 0;
}

static int string_foreign_is_ge(val_t *self, val_t *to) {
    const char *a = (const char *) val_2_intptr(self);
    const char *b = val_2_cstring(to);

    return b ? (strcmp(a, b) >= 0) : 0;
}

static int string_foreign_is_lt(val_t *self, val_t *to) {
    const char *a = (const char *) val_2_intptr(self);
    const char *b = val_2_cstring(to);

    return b ? (strcmp(a, b) < 0) : 0;
}

static int string_foreign_is_le(val_t *self, val_t *to) {
    const char *a = (const char *) val_2_intptr(self);
    const char *b = val_2_cstring(to);

    return b ? (strcmp(a, b) <= 0) : 0;
}

static double value_of_string(val_t *self)
{
    const char *s = val_2_cstring(self);

    return *s ? const_nan.d : 0;
}

static int concat_string(void *hnd, val_t *a, val_t *b, val_t *res)
{
    env_t *env = hnd;

    if (!val_is_string(b)) {
        val_set_nan(res);
        return 0;
    } else {
        int size1 = string_len(a);
        int size2 = string_len(b);
        int len = size1 + size2;
        string_t *s = string_alloc(env, len + 1);

        if (s) {
            // Todo: length overflow should be check! or variable length field.
            memcpy(s->str + 0, val_2_cstring(a), size1);
            memcpy(s->str + size1, val_2_cstring(b), size2 + 1);
            val_set_heap_string(res, (intptr_t) s);
            return 0;
        } else {
            env_set_error(env, ERR_NotEnoughMemory);
            val_set_undefined(res);
            return -ERR_NotEnoughMemory;
        }
    }
}

const val_metadata_t metadata_str_inline = {
    .is_true  = string_inline_is_true,
    .is_equal = string_inline_compare,
    .is_gt    = string_inline_is_gt,
    .is_ge    = string_inline_is_ge,
    .is_lt    = string_inline_is_lt,
    .is_le    = string_inline_is_le,

    .value_of = value_of_string,
    .concat   = concat_string,
};

const val_metadata_t metadata_str_heap = {
    .is_true  = string_heap_is_true,
    .is_equal = string_heap_compare,
    .is_gt    = string_heap_is_gt,
    .is_ge    = string_heap_is_ge,
    .is_lt    = string_heap_is_lt,
    .is_le    = string_heap_is_le,

    .value_of = value_of_string,
    .concat   = concat_string,
};

const val_metadata_t metadata_str_foreign = {
    .is_true  = string_foreign_is_true,
    .is_equal = string_foreign_compare,
    .is_gt    = string_foreign_is_gt,
    .is_ge    = string_foreign_is_ge,
    .is_lt    = string_foreign_is_lt,
    .is_le    = string_foreign_is_le,

    .value_of = value_of_string,
    .concat   = concat_string,
};

