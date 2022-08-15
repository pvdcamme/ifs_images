#include <cstdint>

#ifndef FAST_RANDOM_H
#define FAST_RANDOM_H
struct FastRandom {
    uint64_t s[4];

    FastRandom();
    uint64_t random();
};
#endif
