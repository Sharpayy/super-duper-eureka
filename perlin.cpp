#include "perlin.h"

float PerlinNoiseGenerator::perlin2DConfigurable(float x, float y, int seed, float scale, int octaves, float lacunarity, float persistence) {
    float total = 0.0f;
    float frequency = scale;
    float amplitude = 1.0f;
    float maxAmplitude = 0.0f;

    for (int i = 0; i < octaves; i++) {
        total += perlin2D(x * frequency, y * frequency, seed) * amplitude;

        frequency *= lacunarity;
        maxAmplitude += amplitude;
        amplitude *= persistence;
    }
    return total / maxAmplitude;
}

float PerlinNoiseGenerator::perlin2D(float x, float y, int seed) {
    int x0 = (int)glm::floor(x);
    int x1 = x0 + 1;
    int y0 = (int)glm::floor(y);
    int y1 = y0 + 1;

    float sx = x - (float)x0;
    float sy = y - (float)y0;

    float n0, n1, ix0, ix1, value;

    n0 = dotGridGradient(x0, y0, x, y, seed);
    n1 = dotGridGradient(x1, y0, x, y, seed);
    ix0 = interpolate(n0, n1, sx);

    n0 = dotGridGradient(x0, y1, x, y, seed);
    n1 = dotGridGradient(x1, y1, x, y, seed);
    ix1 = interpolate(n0, n1, sx);

    value = interpolate(ix0, ix1, sy);
    return value;
}

float PerlinNoiseGenerator::interpolate(float a0, float a1, float w) {
    return (a1 - a0) * (3.0 - w * 2.0) * w * w + a0;
}

glm::fvec2 PerlinNoiseGenerator::randomGradient(int ix, int iy, int seed) {
    const unsigned w = 8 * sizeof(unsigned);
    const unsigned s = w / 2;
    unsigned a = ix, b = iy;
    a *= seed; b ^= a << s | a >> w - s;
    b *= seed * 1.27; a ^= b << s | b >> w - s;
    a *= seed * 1.63;
    float random = a * (3.14159265 / ~(~0u >> 1));
    glm::fvec2 v;
    v.x = glm::cos(random); v.y = glm::sin(random);
    return v;
}

float PerlinNoiseGenerator::dotGridGradient(int ix, int iy, float x, float y, int seed) {
    glm::fvec2 gradient = randomGradient(ix, iy, seed);

    float dx = x - (float)ix;
    float dy = y - (float)iy;

    return (dx * gradient.x + dy * gradient.y);
}

