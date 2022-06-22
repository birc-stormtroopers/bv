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
    uint64_t w = v->data[bv_widx(i)];           // Get the word
    return !!((uint64_t)1 & (w >> bv_bidx(i))); // shift the bit down and extract it
}
// Returns itself so we can chain calls. NOTE: this is not an expression
// but a modification, so it has side-effects. Be careful with using this
// in calls where you use the same vector in multiple expressions, since
// the evaluation order is not defined in C.
static inline struct bv *bv_set(struct bv *v, size_t i, bool b)
{
    uint64_t w = v->data[bv_widx(i)]; // Get the word.
    v->data[bv_widx(i)] =
        b                                         // depending on the bit value
            ? (w | (uint64_t)1 << bv_bidx(i))     // mask the bit in
            : (w & ~((uint64_t)1 << bv_bidx(i))); // or mask all the other bits
    return v;
}

struct bv *bv_new(size_t len);                  // new vector all zeros
struct bv *bv_new_from_string(const char *str); // vector spec on form "011010..."
struct bv *bv_copy(struct bv const *v);

// These just return the modified input vector. They return it
// so we can chain operations. NOTE: It is *not* a copy they return
// so be careful with using these in expressions! They have side-effects.
struct bv *bv_zero(struct bv *v);
struct bv *bv_one(struct bv *v);
struct bv *bv_neg(struct bv *v);

struct bv *bv_shift_up(struct bv *v, size_t k);   // v =<< k
struct bv *bv_shift_down(struct bv *v, size_t k); // v =>> k

struct bv *bv_or_assign(struct bv *v, struct bv const *w);  // v |= w
struct bv *bv_and_assign(struct bv *v, struct bv const *w); // v &= w

// These returns copies. They are safe to use in expressions.
struct bv *bv_or(struct bv const *v, struct bv const *w);  // v | w
struct bv *bv_and(struct bv const *v, struct bv const *w); // v & w

bool bv_eq(struct bv const *v, struct bv const *w); // v == w

void bv_print(struct bv const *v);

#endif // BV_H
