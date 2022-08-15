#include <cstdlib>

#include "FastRandom.h"

static uint64_t rol64(uint64_t x, uint64_t k)
{
    return (x << k) | (x >> (64 - k));
}

FastRandom::FastRandom(uint64_t seed) {
    s[0] = seed + 1 + random();
    s[1] = seed + 2 + random();
    s[2] = seed + 3 + random();
    s[3] = seed + 4 + random();
}

uint64_t FastRandom::random()
{
    uint64_t const result = rol64(s[1] * 5, 7) * 9;
    uint64_t const t = s[1] << 17;

    s[2] ^= s[0];
    s[3] ^= s[1];
    s[1] ^= s[2];
    s[0] ^= s[3];

    s[2] ^= t;
    s[3] = rol64(s[3], 45);

    return result;
}
