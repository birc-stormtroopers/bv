#include "bv.h"

#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// MARK: Construction
static inline size_t no_words(size_t no_bits)
{
    // Divide into 64-bit words, rounding up.
    return (no_bits + 63) / 64;
}

struct bv *bv_alloc(size_t no_bits)
{
    size_t header = offsetof(struct bv, data);
    size_t data = sizeof(uint64_t) * no_words(no_bits);
    // Use calloc to satisfy static analysis.
    // It has the added benefit that all new vectors are 0-initialised.
    struct bv *v = calloc(1, header + data);
    assert(v); // We don't handle allocation errors
    v->len = no_bits;
    return v;
}

struct bv *bv_new(size_t len)
{
    return bv_alloc(len);
}

struct bv *bv_new_from_string(const char *str)
{
    size_t len = strlen(str); // FlawFinder: ignore (I know about '\0')
    struct bv *v = bv_alloc(len);
    for (size_t i = 0; *str; i++, str++)
    {
        // set to zero if *str == '0' and one otherwise
        bv_set(v, i, *str != '0');
    }
    return v;
}

struct bv *bv_copy(struct bv const *v)
{
    struct bv *w = bv_alloc(v->len);
    for (size_t i = 0; i < no_words(w->len); i++)
    {
        w->data[i] = v->data[i];
    }
    return w;
}

// MARK Initialisation
void bv_zero(struct bv *v)
{
    for (size_t i = 0; i < no_words(v->len); i++)
    {
        v->data[i] = (uint64_t)0;
    }
}

void bv_one(struct bv *v)
{
    for (size_t i = 0; i < no_words(v->len); i++)
    {
        v->data[i] = ~(uint64_t)0;
    }
}

void bv_neg(struct bv *v)
{
    for (size_t i = 0; i < no_words(v->len); i++)
    {
        v->data[i] = ~v->data[i];
    }
}

// MARK Operations
void bv_shiftl(struct bv *v, size_t k)
{
    // FIXME: doesn't handle k>64
    uint64_t tmp = 0;
    for (size_t i = 0; i < no_words(v->len); i++)
    {
        uint64_t w = (v->data[i] << k) | tmp;
        tmp = v->data[i] >> (64 - k);
        v->data[i] = w;
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

struct bv *bv_or(struct bv const *v, struct bv const *w)
{
    assert(v->len == w->len);
    struct bv *u = bv_alloc(v->len);
    for (size_t i = 0; i < no_words(v->len); i++)
    {
        u->data[i] = v->data[i] | w->data[i];
    }
    return u;
}

struct bv *bv_and(struct bv const *v, struct bv const *w)
{
    assert(v->len == w->len);
    struct bv *u = bv_alloc(v->len);
    for (size_t i = 0; i < no_words(v->len); i++)
    {
        u->data[i] = v->data[i] & w->data[i];
    }
    return u;
}

// MARK I/O
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
            printf("%s", sep[(i / 16) % 4]);
        }
    }
    // Pad the last block with spaces.
    for (size_t i = v->len % 16; i < 16; i++)
    {
        printf(" ");
    }
    printf(" |\n");
}
