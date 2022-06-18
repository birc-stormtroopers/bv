#include "bv.h"

#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
    struct bv *v = calloc(1, bv_size(len));
    assert(v); // We don't handle allocation errors
    v->len = len;
    return v;
}

struct bv *bv_new_from_string(const char *str)
{
    size_t len = strlen(str);
    struct bv *v = calloc(1, bv_size(len));
    assert(v); // We don't handle allocation errors

    v->len = len;

    for (size_t i = 0; *str; i++, str++)
    {
        // set to zero if *str == '0' and one otherwise
        bv_set(v, i, *str != '0');
    }
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

void bv_neg(struct bv *v)
{
    for (size_t i = 0; i < no_words(v->len); i++)
    {
        v->data[i] = ~v->data[i];
    }
}

void bv_or_assign(struct bv *v, struct bv const *w)
{
    assert(v->len == w->len);
    for (size_t i = 0; i < no_words(v->len); i++)
    {
        v->data[i] |= w->data[i];
    }
}

void bv_and_assign(struct bv *v, struct bv const *w)
{
    assert(v->len == w->len);
    for (size_t i = 0; i < no_words(v->len); i++)
    {
        v->data[i] &= w->data[i];
    }
}

void bv_print(struct bv const *v)
{
    static const char *sep[] = {
        " | ", " | ", " | ", " |\n | "};
    uint64_t w;
    printf(" | ");
    for (size_t i = 0; i < v->len; i++)
    {
        if (i % 64 == 0)
        {
            // Fetch next word
            w = v->data[i / 64];
        }
        // Get the next bit and shift the word to get to the
        // one after that
        bool bit = w & 1;
        w >>= 1;

        // Print the bit
        printf("%c", bit ? '1' : '.');

        if ((i + 1) % 16 == 0)
        {
            // Print a separator (between each 16 bits)
            printf("%s", sep[(i / 16) % 8]);
        }
    }
    // Pad the last block with spaces.
    for (size_t i = 0; i < 16 - v->len % 16; i++)
    {
        printf(" ");
    }
    printf(" |\n");
}
