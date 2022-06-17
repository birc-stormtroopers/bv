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

static inline size_t bv_widx(size_t i)
{
    return i >> 6; // i divided by 64
}
static inline size_t bv_bidx(size_t i)
{
    return i & ((1lu << 6) - 1); // i mod 64
}

static inline bool bv_get(struct bv *v, size_t i)
{
    uint64_t w = v->data[bv_widx(i)];
    return !!(1 & (w >> bv_bidx(i)));
}
static inline void bv_set(struct bv *v, size_t i, bool b)
{
    uint64_t w = v->data[bv_widx(i)];
    w = b ? (w | 1lu << bv_bidx(i)) : (w & ~(1lu << bv_bidx(i)));
    v->data[bv_widx(i)] = w;
}

struct bv *bv_new(size_t len);

void bv_zero(struct bv *v);
void bv_one(struct bv *v);

#endif // BV_H
