#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bv.h"

#define sigma 256 // size of alphabet (assumed one byte letters)

static struct bv **build_pattern_masks(size_t m, const char p[m]) // FlawFinder: ignore
{
    struct bv **pmask = malloc(sigma * sizeof *pmask);
    assert(pmask);

    // Build table of all ones
    for (size_t a = 0; a < sigma; a++)
    {
        pmask[a] = bv_set(
            bv_one(bv_new(m)),
            m, 0);
    }

    // Set matches to zero
    for (size_t i = 0; i < m; i++)
    {
        // Set pmatch[a]'s i'th bit to 0 if there is an a
        // at index i in the pattern.
        bv_set(pmask[(unsigned)p[i]], i, 0);
    }

    return pmask;
}

static void free_pattern_masks(struct bv **pmask)
{
    for (size_t a = 0; a < sigma; a++)
    {
        free(pmask[a]);
    }
    free(pmask);
}

int main(int argc, const char *argv[])
{
    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s string pattern\n", argv[0]);
        return 1;
    }

    const char *x = argv[1];
    const char *p = argv[2];
    size_t n = strlen(x); // FlawFinder: ignore
    size_t m = strlen(p); // FlawFinder: ignore

    struct bv **pmask = build_pattern_masks(m, p);
    struct bv *match = bv_one(bv_new(m));

    for (size_t i = 0; i < n; i++)
    {
        // match = (match << 1) | mask[x[i]]
        bv_or_assign(
            bv_shift_up(match, 1),
            pmask[(unsigned)x[i]]);

        if (bv_get(match, m - 1) == 0)
        {
            printf("match at: %lu\n", i - m + 1);
        }

        // Print state for educational purposes...
        printf("%c ", x[i]);
        bv_print(match);
    }

    free(match);
    free_pattern_masks(pmask);

    return 0;
}
