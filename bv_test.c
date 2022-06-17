#include "bv.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

static void test_creation(void)
{
    struct bv *v = bv_new(66);
    for (size_t i = 0; i < 66; i++)
    {
        assert(bv_get(v, i) == false);
    }
    bv_one(v);
    for (size_t i = 0; i < 66; i++)
    {
        assert(bv_get(v, i) == true);
    }
}

static void test_set(void)
{
    struct bv *v = bv_new(66);
    for (size_t i = 0; i < 66; i++)
    {
        assert(bv_get(v, i) == false);
    }
    for (size_t i = 0; i < 66; i++)
    {
        bv_set(v, i, i % 2);
    }
    for (size_t i = 0; i < 66; i++)
    {
        printf("%lu: %d vs %d\n", i, bv_get(v, i), !!(i % 2));
        assert(bv_get(v, i) == !!(i % 2));
    }
}

int main(void)
{
    test_creation();
    test_set();

    return 0;
}
