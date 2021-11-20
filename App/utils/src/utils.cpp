#include "utils.hpp"
#include <stdint.h>

int inline fastfloor(float fp) {
    int i = static_cast<int>(fp);
    return (fp < i) ? (i - 1) : (i);
}

static float grad(int32_t hash, float x) {
    const int32_t h = hash & 0x0F;
    float grad = 1.0f + (h & 7);
    if ((h & 8) != 0)
        grad = -grad;
    return (grad * x);
}

static inline uint8_t hash(int32_t i) { return perm[static_cast<uint8_t>(i)]; }

float simplexNoise(float x) {
    float n0, n1;
    int32_t i0 = fastfloor(x);
    int32_t i1 = i0 + 1;
    float x0 = x - i0;
    float x1 = x0 - 1.0f;
    float t0 = 1.0f - x0 * x0;
    t0 *= t0;
    n0 = t0 * t0 * grad(hash(i0), x0);
    float t1 = 1.0f - x1 * x1;
    t1 *= t1;
    n1 = t1 * t1 * grad(hash(i1), x1);
    return 0.395f * (n0 + n1);
}
