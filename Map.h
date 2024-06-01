#pragma once
#include "Bezier.h"
#include "perlin.h"

class Map {
public:
	Map() = default;
	Map(int map_w, int map_h, double _persistence, double _frequency, double _amplitude, int _octaves, int _randomseed) {
		this->map_w = map_w;
		this->map_h = map_h;
		data = nullptr;
		generateMap(_persistence, _frequency, _amplitude, _octaves, _randomseed);

	}

	Texture2D getMap() {
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
	void generateMap(double _persistence, double _frequency, double _amplitude, int _octaves, int _randomseed) {
		int size = map_w * map_h;
		char* tileMap = new char[size * 3];

		PerlinNoiseGenerator perlin{ _persistence, _frequency, _amplitude, _octaves, _randomseed,};
		double dx, dy;
		int idx = 0;
		for (int y = 0; y < map_h; y++) {
			for (int x = 0; x < map_w; x++) {
				idx = (y * map_w + x) * 3;
				dx = x / (double)map_w;
				dy = y / (double)map_h;
				//float val = (perlin.perlin2DConfigurable(x, y, seed, scale, octaves, lacunarity, persistence) * 0.5f + 0.5f) * 255.0f;
				float val = (perlin.GetHeight(dx, dy) * 0.5f + 0.5f) * 255.0f;

				//std::cout << val << "\n";
				//MOUNTAIN
				if (val > 0.75f * 255.0f) {
					// MOUNTAIN
					tileMap[idx] = (char)139.0f;
					tileMap[idx + 1] = (char)137.0f;
					tileMap[idx + 2] = (char)137.0f;
				}
				else if (val > 0.5f * 255.0f) {
					// HILL
					tileMap[idx] = (char)34.0f;
					tileMap[idx + 1] = (char)139.0f;
					tileMap[idx + 2] = (char)34.0f;
				}
				else if (val > 0.3f * 255.0f) {
					// GRASSLAND
					tileMap[idx] = (char)34.0f;
					tileMap[idx + 1] = (char)139.0f;
					tileMap[idx + 2] = (char)34.0f;
				}
				else if (val > 0.15f * 255.0f) {
					// BEACH
					tileMap[idx] = (char)238.0f;
					tileMap[idx + 1] = (char)214.0f;
					tileMap[idx + 2] = (char)175.0f;
				}
				else {
					// WATER
					tileMap[idx] = (char)0.0f;
					tileMap[idx + 1] = (char)105.0f;
					tileMap[idx + 2] = (char)148.0f;
				}

				//tileMap[idx] = /*(char)((perlin.perlin2DConfigurable(x / (float)map_w, y / (float)map_h, 1221143, 1.0f, 8, 3.0f, 0.65f) * 0.5f + 0.5f) * 255.0f);*/
			}
		}
		map = Texture2D{ tileMap, (int)map_w, (int)map_h , GL_RGB , GL_RGBA };
		map.genMipmap();
		data = tileMap;
	}
	int map_w;
	int map_h;

private:
	Texture2D map;
	char* data;
};