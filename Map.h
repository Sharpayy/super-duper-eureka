#pragma once
#include "Bezier.h"
#include "perlin.h"

class Map {
private:

	Texture2D* map;
public:
	Map() = default;
	Map(float tile_w, float tile_h, float map_w, float map_h, int seed, float scale, int octaves, float lacunarity, float persistence) {
		this->tile_w = tile_w;
		this->tile_h = tile_h;
		this->map_w = map_w;
		this->map_h = map_h;
		generateMap(seed, scale, octaves, lacunarity, persistence);

	}

	void generateMap(int seed, float scale, int octaves, float lacunarity, float persistence ) {
		int elementsX = map_w / tile_w;
		int elementsY = map_h / tile_h;
		int size = elementsX * elementsY;
		float* tileMap = new float[size];

		PerlinNoiseGenerator perlin;
		
		int idx = 0;
		for (int y = 0; y < elementsY; y++) {
			for (int x = 0; x < elementsX; x++) {
				tileMap[y * elementsX + x] = perlin.perlin2DConfigurable(x, y, seed, scale, octaves, lacunarity, persistence);
			}
		}
		map = new Texture2D{ tileMap, (int)map_w, (int)map_h , GL_R32F , GL_RGBA };

	}

	float GetMapValue(int idx, int idy) {
		
	}

	float tile_w;
	float tile_h;
	float map_w;
	float map_h;

};