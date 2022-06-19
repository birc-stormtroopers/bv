#ifndef BV_H
#define BV_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

struct bv
{
    size_t len;
    uint64_t data[];
};

// Any optimising compiler will work out that / and % can be translated into
// bit shifts and masking when it knows the divisor and the divisor is a power
// of two.

// clang-format off
static inline size_t bv_widx(size_t i) { return i / 64; }
static inline size_t bv_bidx(size_t i) { return i % 64; }
// clang-format on

static inline bool bv_get(struct bv *v, size_t i)
{
    uint64_t w = v->data[bv_widx(i)]; // Get the word
    return !!(1 & (w >> bv_bidx(i))); // shift the bit down and extract it
}
static inline void bv_set(struct bv *v, size_t i, bool b)
{
    uint64_t w = v->data[bv_widx(i)]; // Get the word.
    v->data[bv_widx(i)] =
        b                                 // depending on the bit value
            ? (w | 1lu << bv_bidx(i))     // mask the bit in
            : (w & ~(1lu << bv_bidx(i))); // or mask all the other bits
}

struct bv *bv_new(size_t len);                  // new vector all zeros
struct bv *bv_new_from_string(const char *str); // vector spec on form "011010..."
struct bv *bv_copy(struct bv const *v);

void bv_zero(struct bv *v);
void bv_one(struct bv *v);
void bv_neg(struct bv *v);

void bv_shiftl(struct bv *v, size_t k);

void bv_or_assign(struct bv *v, struct bv const *w);  // v |= w
void bv_and_assign(struct bv *v, struct bv const *w); // v &= w

struct bv *bv_or(struct bv const *v, struct bv const *w);  // v | w
struct bv *bv_and(struct bv const *v, struct bv const *w); // v & w

void bv_print(struct bv const *v);

#endif // BV_H
