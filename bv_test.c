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
    free(v);
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

    struct bv *w = bv_copy(v);
    assert(bv_get(w, 0) == 1);
    assert(bv_get(w, 1) == 0);
    assert(bv_get(w, 2) == 0);
    assert(bv_get(w, 3) == 1);

    bv_neg(v);
    assert(bv_get(v, 0) == 0);
    assert(bv_get(v, 1) == 1);
    assert(bv_get(v, 2) == 1);
    assert(bv_get(v, 3) == 0);
    assert(bv_get(w, 0) == 1);
    assert(bv_get(w, 1) == 0);
    assert(bv_get(w, 2) == 0);
    assert(bv_get(w, 3) == 1);

    free(v);
    free(w);
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

static void test_shift_up(void)
{
    struct bv *v = bv_new(150);
    bv_set(v, 0, 1);
    bv_shift_up(v, 0);
    struct bv *test = bv_new_from_string(
        // word 0
        "1000000000000000" // 0..15
        "0000000000000000" // 16..31
        "0000000000000000" // 32..47
        "0000000000000000" // 48..63
        // word 1
        "0000000000000000" // 64..79
        "0000000000000000" // 80..95
        "0000000000000000" // 96..111
        "0000000000000000" // 112..127
        // word 3
        "0000000000000000" // 128..143
        "000000"           // 144..149
    );
    assert(bv_eq(v, test));
    free(test);

    bv_shift_up(v, 1);
    test = bv_new_from_string(
        // word 0
        "0100000000000000" // 0..15
        "0000000000000000" // 16..31
        "0000000000000000" // 32..47
        "0000000000000000" // 48..63
        // word 1
        "0000000000000000" // 64..79
        "0000000000000000" // 80..95
        "0000000000000000" // 96..111
        "0000000000000000" // 112..127
        // word 3
        "0000000000000000" // 128..143
        "000000"           // 144..149
    );
    assert(bv_eq(v, test));
    free(test);

    bv_set(v, 0, 1);
    bv_shift_up(v, 2);
    test = bv_new_from_string(
        // word 0
        "0011000000000000" // 0..15
        "0000000000000000" // 16..31
        "0000000000000000" // 32..47
        "0000000000000000" // 48..63
        // word 1
        "0000000000000000" // 64..79
        "0000000000000000" // 80..95
        "0000000000000000" // 96..111
        "0000000000000000" // 112..127
        // word 3
        "0000000000000000" // 128..143
        "000000"           // 144..149
    );
    assert(bv_eq(v, test));
    free(test);

    bv_set(v, 0, 1);
    bv_shift_up(v, 4);
    test = bv_new_from_string(
        // word 0
        "0000101100000000" // 0..15
        "0000000000000000" // 16..31
        "0000000000000000" // 32..47
        "0000000000000000" // 48..63
        // word 1
        "0000000000000000" // 64..79
        "0000000000000000" // 80..95
        "0000000000000000" // 96..111
        "0000000000000000" // 112..127
        // word 3
        "0000000000000000" // 128..143
        "000000"           // 144..149
    );
    assert(bv_eq(v, test));
    free(test);

    bv_set(v, 0, 1);
    bv_shift_up(v, 8);
    test = bv_new_from_string(
        // word 0
        "0000000010001011" // 0..15
        "0000000000000000" // 16..31
        "0000000000000000" // 32..47
        "0000000000000000" // 48..63
        // word 1
        "0000000000000000" // 64..79
        "0000000000000000" // 80..95
        "0000000000000000" // 96..111
        "0000000000000000" // 112..127
        // word 3
        "0000000000000000" // 128..143
        "000000"           // 144..149
    );
    assert(bv_eq(v, test));
    free(test);

    bv_set(v, 0, 1);
    bv_shift_up(v, 16);
    test = bv_new_from_string(
        // word 0
        "0000000000000000" // 0..15
        "1000000010001011" // 16..31
        "0000000000000000" // 32..47
        "0000000000000000" // 48..63
        // word 1
        "0000000000000000" // 64..79
        "0000000000000000" // 80..95
        "0000000000000000" // 96..111
        "0000000000000000" // 112..127
        // word 3
        "0000000000000000" // 128..143
        "000000"           // 144..149
    );
    assert(bv_eq(v, test));
    free(test);

    bv_set(v, 0, 1);
    bv_shift_up(v, 32);
    test = bv_new_from_string(
        // word 0
        "0000000000000000" // 0..15
        "0000000000000000" // 16..31
        "1000000000000000" // 32..47
        "1000000010001011" // 48..63
        // word 1
        "0000000000000000" // 64..79
        "0000000000000000" // 80..95
        "0000000000000000" // 96..111
        "0000000000000000" // 112..127
        // word 3
        "0000000000000000" // 128..143
        "000000"           // 144..149
    );
    assert(bv_eq(v, test));
    free(test);

    bv_set(v, 0, 1);
    bv_shift_up(v, 64);
    test = bv_new_from_string(
        // word 0
        "0000000000000000" // 0..15
        "0000000000000000" // 16..31
        "0000000000000000" // 32..47
        "0000000000000000" // 48..63
        // word 1
        "1000000000000000" // 64..79
        "0000000000000000" // 80..95
        "1000000000000000" // 96..111
        "1000000010001011" // 112..127
        // word 3
        "0000000000000000" // 128..143
        "000000"           // 144..149
    );
    assert(bv_eq(v, test));
    free(test);

    bv_set(v, 0, 1);
    bv_shift_up(v, 96);
    test = bv_new_from_string(
        // word 0
        "0000000000000000" // 0..15
        "0000000000000000" // 16..31
        "0000000000000000" // 32..47
        "0000000000000000" // 48..63
        // word 1
        "0000000000000000" // 64..79
        "0000000000000000" // 80..95
        "1000000000000000" // 96..111
        "0000000000000000" // 112..127
        // word 3
        "0000000000000000" // 128..143
        "000000"           // 144..149
    );
    assert(bv_eq(v, test));

    test = bv_new(150);
    for (size_t i = 0; i < 150; i++)
    {
        bv_zero(v);
        bv_zero(test);

        bv_set(v, 0, 1);
        bv_shift_up(v, i);
        bv_set(test, i, 1);

        assert(bv_eq(v, test));
    }

    free(test);
    free(v);
}

static void test_shift_down(void)
{
    struct bv *v = bv_new(150);
    bv_set(v, 149, 1);
    bv_shift_down(v, 0);
    struct bv *test = bv_new_from_string(
        // word 0
        "0000000000000000" // 0..15
        "0000000000000000" // 16..31
        "0000000000000000" // 32..47
        "0000000000000000" // 48..63
        // word 1
        "0000000000000000" // 64..79
        "0000000000000000" // 80..95
        "0000000000000000" // 96..111
        "0000000000000000" // 112..127
        // word 3
        "0000000000000000" // 128..143
        "000001"           // 144..149
    );
    assert(bv_eq(v, test));
    free(test);

    bv_shift_down(v, 1);
    test = bv_new_from_string(
        // word 0
        "0000000000000000" // 0..15
        "0000000000000000" // 16..31
        "0000000000000000" // 32..47
        "0000000000000000" // 48..63
        // word 1
        "0000000000000000" // 64..79
        "0000000000000000" // 80..95
        "0000000000000000" // 96..111
        "0000000000000000" // 112..127
        // word 3
        "0000000000000000" // 128..143
        "000010"           // 144..149
    );
    assert(bv_eq(v, test));
    free(test);

    bv_set(v, 149, 1);
    bv_shift_down(v, 2);
    test = bv_new_from_string(
        // word 0
        "0000000000000000" // 0..15
        "0000000000000000" // 16..31
        "0000000000000000" // 32..47
        "0000000000000000" // 48..63
        // word 1
        "0000000000000000" // 64..79
        "0000000000000000" // 80..95
        "0000000000000000" // 96..111
        "0000000000000000" // 112..127
        // word 3
        "0000000000000000" // 128..143
        "001100"           // 144..149
    );
    assert(bv_eq(v, test));
    free(test);

    bv_set(v, 149, 1);
    bv_shift_down(v, 4);
    test = bv_new_from_string(
        // word 0
        "0000000000000000" // 0..15
        "0000000000000000" // 16..31
        "0000000000000000" // 32..47
        "0000000000000000" // 48..63
        // word 1
        "0000000000000000" // 64..79
        "0000000000000000" // 80..95
        "0000000000000000" // 96..111
        "0000000000000000" // 112..127
        // word 3
        "0000000000000011" // 128..143
        "010000"           // 144..149
    );
    assert(bv_eq(v, test));
    free(test);

    bv_set(v, 149, 1);
    bv_shift_down(v, 8);
    test = bv_new_from_string(
        // word 0
        "0000000000000000" // 0..15
        "0000000000000000" // 16..31
        "0000000000000000" // 32..47
        "0000000000000000" // 48..63
        // word 1
        "0000000000000000" // 64..79
        "0000000000000000" // 80..95
        "0000000000000000" // 96..111
        "0000000000000000" // 112..127
        // word 3
        "0000001101000100" // 128..143
        "000000"           // 144..149
    );
    assert(bv_eq(v, test));
    free(test);

    test = bv_new(150);
    for (size_t i = 0; i < 150; i++)
    {
        bv_zero(v);
        bv_zero(test);

        bv_set(v, 149, 1);
        bv_shift_down(v, i);
        bv_set(test, 149 - i, 1);

        assert(bv_eq(v, test));
    }

    free(test);
    free(v);
}

int main(void)
{
    test_creation();
    test_set();
    test_neg();
    test_or();
    test_and();
    test_shift_up();
    test_shift_down();

    return 0;
}
