#include "bv.h"

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>

static inline size_t no_words(size_t no_bits)
{
    // Divide into 64-bit words, rounding up.
    return (no_bits + 63) / 64;
}

static inline size_t bv_size(size_t no_bits)
{
    size_t header = offsetof(struct bv, data);
    size_t data = sizeof(uint64_t) * no_words(no_bits);
    return header + data;
}

struct bv *bv_new(size_t len)
{
    struct bv *v = malloc(bv_size(len));
    assert(v); // We don't handle allocation errors

    v->len = len;
    bv_zero(v);
    return v;
}

void bv_zero(struct bv *v)
{
    for (size_t i = 0; i < no_words(v->len); i++)
    {
        v->data[i] = 0llu;
    }
}

void bv_one(struct bv *v)
{
    for (size_t i = 0; i < no_words(v->len); i++)
    {
        v->data[i] = ~0llu;
    }
}
