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
void bv_shiftl(struct bv *v, size_t m)
{
    size_t k = m % 64;
    size_t offset = m / 64;

    if (k == 0)
    {
        // If k is zero we are moving whole words, and we might as well
        // do that the easy way. In any case, the general code uses
        // w >> (64 - k) which would be w >> 64 which is undefined for
        // 64-bit words, so even if we took the complicated solution
        // it wouldn't work for k == 0.
        for (size_t i = 0; i < (no_words(v->len) - offset); i++)
        {
            size_t ii = no_words(v->len) - i - 1;          // where we copy to
            size_t jj = no_words(v->len) - i - offset - 1; // where we copy from
            v->data[ii] = v->data[jj];
        }
    }
    else
    {
        for (size_t i = 0; i < (no_words(v->len) - offset); i++)
        {
            size_t ii = no_words(v->len) - i - 1;          // where we copy to
            size_t jj = no_words(v->len) - i - offset - 1; // where we copy from
            // u          w
            // [....[xxx]][[yyyy]...] as bitvector, but as words
            // [[xxx]....][...[yyyy]] (remember words read right-to-left, vector left-to-right)
            // u >> (64 - k): [ 0000 [xxx]]
            // w << k:        [[yyyy] 000 ]
            // u | w:         [ yyyy  xxx ]
            // as bitvector:  [ xxx  yyyy ]
            uint64_t u = (jj > 0) ? (v->data[jj - 1] >> (64 - k)) : 0;
            uint64_t w = (v->data[jj] << k);
            v->data[ii] = u | w;
        }
    }

    // zero the lower words, simulating that we shifted the bits up.
    for (size_t i = 0; i < offset; i++)
    {
        v->data[i] = 0;
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

bool bv_eq(struct bv const *v, struct bv const *w)
{
    if (v->len != w->len)
        return false;

    // Compare the first no_words - 1 as full words.
    size_t i = 0;
    for (; i < no_words(v->len) - 1; i++)
    {
        if (v->data[i] != w->data[i])
            return false;
    }

    // For the last word, only compare the k first bits.
    // The ? operator because the full word, k=64, becomes k=0
    // with modulus.
    size_t k = v->len % 64;
    uint64_t mask = (k == 0) ? ~0 : (1 << k) - 1;
    return (v->data[i] & mask) == (w->data[i] & mask);
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
