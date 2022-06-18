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

    //free(v); -- FIXME testing
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
        assert(bv_get(v, i) == !!(i % 2));
    }

    free(v);
}

static void test_neg(void)
{
    struct bv *v = bv_new_from_string("1001");
    assert(v->len == 4);

    assert(bv_get(v, 0) == 1);
    assert(bv_get(v, 1) == 0);
    assert(bv_get(v, 2) == 0);
    assert(bv_get(v, 3) == 1);

    bv_neg(v);
    assert(bv_get(v, 0) == 0);
    assert(bv_get(v, 1) == 1);
    assert(bv_get(v, 2) == 1);
    assert(bv_get(v, 3) == 0);

    free(v);
}

static void test_or(void)
{
    struct bv *v = bv_new_from_string("100100");
    assert(v->len == 6);
    struct bv *w = bv_new_from_string("110011");
    assert(w->len == 6);

    bv_or_assign(v, w);
    assert(bv_get(v, 0) == 1);
    assert(bv_get(v, 1) == 1);
    assert(bv_get(v, 2) == 0);
    assert(bv_get(v, 3) == 1);
    assert(bv_get(v, 4) == 1);
    assert(bv_get(v, 5) == 1);

    free(v);
    free(w);
}

static void test_and(void)
{
    struct bv *v = bv_new_from_string("100100");
    assert(v->len == 6);
    struct bv *w = bv_new_from_string("110011");
    assert(w->len == 6);

    bv_and_assign(v, w);
    assert(bv_get(v, 0) == 1);
    assert(bv_get(v, 1) == 0);
    assert(bv_get(v, 2) == 0);
    assert(bv_get(v, 3) == 0);
    assert(bv_get(v, 4) == 0);
    assert(bv_get(v, 5) == 0);

    free(v);
    free(w);
}

int main(void)
{
    test_creation();
    test_set();
    test_neg();
    test_or();
    test_and();

    return 0;
}
