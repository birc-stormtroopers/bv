#include "bv.h"

#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Shifting uint64_t by 64 or more is undefined behaviour in C, but we want such
// shifts to give us zero. It removes special cases here and there, not having to
// check if k is 0 when doing someting like (w >> (64 - k)).
#define RSHIFT(W, K) (((K) < 64) ? ((W) >> (K)) : 0)
#define LSHIFT(W, K) (((K) < 64) ? ((W) << (K)) : 0)

#define NWORDS(VEC) no_words((VEC)->len)

// The word at the current index
#define WORD(VEC) ((VEC)->data[i_])

// Word offset to the left of the current index.
#define WORD_BEFORE(VEC, OFFSET) \
    (((OFFSET) <= i_) ? ((VEC)->data[i_ - (OFFSET)]) : (uint64_t)0)
// Word offset to the right of the current index.
#define WORD_AFTER(VEC, OFFSET) \
    ((((OFFSET) + i_) < NWORDS(VEC)) ? ((VEC)->data[i_ + (OFFSET)]) : (uint64_t)0)

// clang-format off
#define EACH_WORD(VEC, ...)            EACH_WORD_FROM(VEC, 0, __VA_ARGS__)
#define EACH_WORD_FROM(VEC, FROM, ...) EACH_WORD_RANGE(VEC, FROM, NWORDS(VEC), __VA_ARGS__)
#define EACH_WORD_TO(VEC, TO, ...)     EACH_WORD_RANGE(VEC, 0, TO, __VA_ARGS__)
#define EACH_WORD_RANGE(VEC, FROM, TO, ...)   \
    for (size_t i_ = (FROM); i_ < (TO); i_++) \
    {                                         \
        __VA_ARGS__;                          \
    }
// clang-format on

#define EACH_WORD_REV_TO(VEC, TO, ...)                      \
    for (size_t ii_ = 0; ii_ < (NWORDS(VEC) - (TO)); ii_++) \
    {                                                       \
        size_t i_ = NWORDS(VEC) - ii_ - 1;                  \
        __VA_ARGS__;                                        \
    }

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
    EACH_WORD(v, WORD(w) = WORD(v));
    return w;
}

// MARK Initialisation
void bv_zero(struct bv *v)
{
    EACH_WORD(v, WORD(v) = (uint64_t)0);
}

void bv_one(struct bv *v)
{
    EACH_WORD(v, WORD(v) = ~(uint64_t)0);
}

void bv_neg(struct bv *v)
{
    EACH_WORD(v, WORD(v) = ~WORD(v));
}

// A vector is "dirty" if there are set bits in the last word, beyond the
// last bit. This can happen with shifts, but it complicates some computations
// if we have to mask those out for comparisons or shifts. In those case,
// we should "clean" the vector first.
static void bv_clean(struct bv *v)
{
    size_t k = v->len % 64;
    if (k != 0) // if k == 0 there are no extra bits.
    {
        uint64_t mask = (1 << k) - 1;          // lower k words; we want to keep them.
        v->data[no_words(v->len) - 1] &= mask; // remove the other bits.
    }
}

// MARK Operations

void bv_shift_up(struct bv *v, size_t m)
{
    size_t k = m % 64;
    size_t offset = m / 64;

    // From offset and up, we shift and or to get the bit patterns,
    // going through the words in reverse.
    // clang-format off
    EACH_WORD_REV_TO(v, offset, {
        uint64_t u = WORD_BEFORE(v, offset + 1);
        uint64_t w = WORD_BEFORE(v, offset);
        WORD(v) = RSHIFT(u, 64 - k) | LSHIFT(w, k);
    })
    // clang-format on

    // zero the lower words, simulating that we shifted the bits up.
    EACH_WORD_TO(v, offset, WORD(v) = 0);

    // clean up the bits we might have shifted beyond the end
    bv_clean(v);
}

void bv_shift_down(struct bv *v, size_t m)
{
    size_t k = m % 64;
    size_t offset = m / 64;

    // From zero up to (n - offset) we shift and or to get the bit patterns.
    // clang-format off
    EACH_WORD_TO(v, NWORDS(v) - offset, {
        uint64_t u = WORD_AFTER(v, offset);
        uint64_t w = WORD_AFTER(v, offset + 1);
        WORD(v) = RSHIFT(u, k) | LSHIFT(w, 64 - k);
    })
    // clang-format on

    // zero the upper words, simulating that we shifted the bits down.
    EACH_WORD_REV_TO(v, NWORDS(v) - offset, WORD(v) = 0);

    // clean up the bits we might have shifted beyond the end
    bv_clean(v);
}

void bv_or_assign(struct bv *v, struct bv const *w)
{
    assert(v->len == w->len);
    EACH_WORD(v, WORD(v) |= WORD(w));
}

void bv_and_assign(struct bv *v, struct bv const *w)
{
    assert(v->len == w->len);
    EACH_WORD(v, WORD(v) &= WORD(w));
}

struct bv *bv_or(struct bv const *v, struct bv const *w)
{
    assert(v->len == w->len);
    struct bv *u = bv_alloc(v->len);
    EACH_WORD(u, WORD(u) = WORD(v) | WORD(w));
    return u;
}

struct bv *bv_and(struct bv const *v, struct bv const *w)
{
    assert(v->len == w->len);
    struct bv *u = bv_alloc(v->len);
    EACH_WORD(u, WORD(u) = WORD(v) & WORD(w));
    return u;
}

bool bv_eq(struct bv const *v, struct bv const *w)
{
    if (v->len != w->len)
        return false;
    EACH_WORD(v, if (WORD(v) != WORD(w)) return false);
    return true;
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
