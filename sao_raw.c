// Implementing SHIFT-and-OR using raw machine words instead of the
// struct bv type. This is more efficient, but less dynamic. It limits
// the pattern to the chosen word size (here 64-bit, so pattern length
// is limited to 64).

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define word uint64_t
#define WORD_SIZE 64
#define sigma 256 // size of alphabet (assumed one byte letters)

// For educational purposes
static void print_bits(word w, size_t m)
{
    for (size_t i = 0; i < m; i++, w >>= 1)
    {
        printf("%llu", w & (word)1);
    }
    putchar('\n');
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

    if (m >= WORD_SIZE)
    {
        fprintf(stderr, "Pattern too long.\n");
        return 1;
    }

    word *pmask = malloc(sigma * sizeof *pmask);
    assert(pmask); // abort on allocation failure (unlikely as it is).

    // Set all bits to 1. memset() does it for bytes, but if we set all the
    // bytes to 0xff then we also set all the bits to one.
    memset(pmask, 0xff, sigma * sizeof *pmask);

    // Set matches to zero
    word match = (word)1; // Moves 1 up through the length of p
    for (size_t i = 0; i < m; i++, match <<= 1)
    {
        // Set pmatch[a]'s i'th bit to 0 if there is an a
        // at index i in the pattern.
        pmask[(unsigned)p[i]] &= ~match; // set the match-bit to zero
    }

    match = ~(word)0;                    // state vector through the scan
    word check_bit = (word)1 << (m - 1); // (m-1)'th bit for checking matches
    for (size_t i = 0; i < n; i++)
    {
        match = (match << 1) | pmask[(unsigned)x[i]];

        if ((match & check_bit) == 0)
        {
            printf("match at: %lu\n", i - m + 1);
        }

        // Print state for educational purposes...
        printf("%c ", x[i]);
        print_bits(match, m);
    }

    free(pmask);

    return 0;
}
