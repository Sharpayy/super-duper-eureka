#pragma once
#include <glm.hpp>

class PerlinNoiseGenerator {
public:
    float perlin2DConfigurable(float x, float y, int seed, float scale, int octaves, float lacunarity, float persistence);

    float perlin2D(float x, float y, int seed);

private:
    float interpolate(float a0, float a1, float w);

    glm::fvec2 randomGradient(int ix, int iy, int seed);

    float dotGridGradient(int ix, int iy, float x, float y, int seed);
};
