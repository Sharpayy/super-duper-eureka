#pragma once
#include "Bezier.h"
#include "perlin.h"

class Map {
public:
	Map() = default;
	Map(int map_w, int map_h, int seed, float scale, int octaves, float lacunarity, float persistence) {
		this->map_w = map_w;
		this->map_h = map_h;
		map = nullptr;
		data = nullptr;
		generateMap(seed, scale, octaves, lacunarity, persistence);

	}

	Texture2D* getMap() {
		return map;
	}

	bool isTileAccesible(int idx, float min, float max) {
		float val = data[idx];
		return (min >= min && val <= max);
	}

	void removeData() {
		//delete data;
		data = nullptr;
	}

private:
	void generateMap(int seed, float scale, int octaves, float lacunarity, float persistence ) {
		int size = map_w * map_h;
		float* tileMap = new float[size];

		PerlinNoiseGenerator perlin;
		
		int idx = 0;
		for (int y = 0; y < map_h; y++) {
			for (int x = 0; x < map_w; x++) {
				idx = y * map_w + x;
				tileMap[idx] = 0.7f;/* perlin.perlin2DConfigurable(x, y, seed, scale, octaves, lacunarity, persistence);*/
			}
		}
		map = new Texture2D{ tileMap, (int)map_w, (int)map_h , GL_R32F , GL_RGBA };
		data = tileMap;
	}
	int map_w;
	int map_h;

private:
	Texture2D* map;
	float* data;
};