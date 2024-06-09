#pragma once
#include "Bezier.h"
#include "perlin.h"
#include <iostream>
#include <vector>
#include <algorithm>


#define MOUNTAIN 0
#define HILL 1
#define FOREST 2
#define TAIGA 3
#define TUNDRA 4
#define SAVANNA 5
#define GRASSLAND 6
#define PLAINS 7
#define SWAMP 8
#define BEACH 9
#define JUNGLE 10
#define DESERT 11
#define SNOW 12
#define WATER 13
#define LAKE 14
#define RIVER 15
#define STREAM 16
#define BROOK 17
#define POND 18
#define CREEK 19

//Biome{ MOUNTAIN, 191.25f, (char)139, (char)137, (char)137 },
//Biome{ HILL, 153.5f, (char)100, (char)100, (char)100 },
//Biome{ GRASSLAND, 76.5f, (char)34, (char)139, (char)34 },
//Biome{ BEACH, 38.25f, (char)238, (char)214, (char)175 },
//Biome{ WATER, 0.0f, (char)0, (char)105, (char)148 },
//Biome{ DESERT, 51.0f, (char)237, (char)201, (char)175 },
//Biome{ FOREST, 102.0f, (char)34, (char)139, (char)34 },
//Biome{ SWAMP, 25.5f, (char)47, (char)79, (char)79 },
//Biome{ TUNDRA, 140.5f, (char)176, (char)196, (char)222 },
//Biome{ SAVANNA, 95.5f, (char)0, (char)128, (char)0 },
//Biome{ TAIGA, 127.0f, (char)0, (char)100, (char)0 }

class Map {
public:
	Map() = default;
	Map(float map_w, float map_h, double _persistence, double _frequency, double _amplitude, int _octaves, int _randomseed) {
		this->map_w = map_w;
		this->map_h = map_h;
		data = nullptr;

		biomes = {
			Biome{ MOUNTAIN, 191.25f, (char)139, (char)137, (char)137 },
			Biome{ HILL, 155.5f, (char)100, (char)100, (char)100 },
			Biome{ GRASSLAND, 76.5f, (char)34, (char)139, (char)34 },
			Biome{ BEACH, 38.25f, (char)238, (char)214, (char)175 },
			Biome{ WATER, 0.0f, (char)0, (char)105, (char)148 },
			Biome{ DESERT, 51.0f, (char)237, (char)201, (char)175 },
			Biome{ SNOW, 200.0f, (char)255, (char)250, (char)250 },
			Biome{ SWAMP, 25.5f, (char)47, (char)79, (char)79 },
			Biome{ TUNDRA, 125.5f, (char)0, (char)90, (char)0 },
			Biome{ SAVANNA, 100.5f, (char)0, (char)110, (char)0 },
			//Biome{ TAIGA, 153.0f, (char)120, (char)100, (char)0 }
		};
		sortBiomes();
		generateMapData(_persistence, _frequency, _amplitude, _octaves, _randomseed);
	}

	void createMap() {
		map = Texture2D{ data, (int)map_w, (int)map_h , GL_RGB , GL_RGBA };
		map.genMipmap();
		delete data;
	}

	Texture2D getMap() {
		return map;
	}

	char* getData() {
		return data;
	}

	//char* GetTile(float x, float y, float scale)
	//{
	//	float true_x = (x * scale) + (map_w * scale / 2.0f);
	//	float true_y = (y * scale) + (map_h * scale / 2.0f);

	//	int idx = ((int)true_y * 3 * map_w) + ((int)true_x * 3);

	//	return data + idx;
	//}

	uint8_t getBiomeType(float x, float y, float scale) {
		float true_x = (x * scale) + (map_w  / 2.0f);
		float true_y = (y * scale) + (map_h  / 2.0f);
		int idx = ((int)true_y * 3 * map_w) + ((int)true_x * 3);

		char c1, c2, c3;
		char* data = getData();
		c1 = data[idx];
		c2 = data[idx + 1];
		c3 = data[idx + 2];
		for (auto& biome : biomes) {
			if (biome.compare(c1, c2, c3)) {
				return biome.type;
			}
		}
		return -1;
	}

	uint8_t getBiomeThr(float x, float y, float scale) {
		float true_x = (x * scale) + (map_w / 2.0f);
		float true_y = (y * scale) + (map_h / 2.0f);
		int idx = ((int)true_y * 3 * map_w) + ((int)true_x * 3);

		char c1, c2, c3;
		char* data = getData();
		c1 = data[idx];
		c2 = data[idx + 1];
		c3 = data[idx + 2];
		for (auto& biome : biomes) {
			if (biome.compare(c1, c2, c3)) {
				return biome.thr;
			}
		}
		return -1;
	}

	bool isTileAccesible(int idx, float min, float max) {
		float val = data[idx];
		return (min >= min && val <= max);
	}

private:
	void generateMapData(double _persistence, double _frequency, double _amplitude, int _octaves, int _randomseed) {
		int size = map_w * map_h;
		char* tileMap = new char[size * 3];

		PerlinNoiseGenerator perlin{ _persistence, _frequency, _amplitude, _octaves, _randomseed,};
		double dx, dy;
		int idx;
		double val;
		double r = 255.0;
		for (int y = 0; y < map_h; y++) {
			for (int x = 0; x < map_w; x++) {
				//idx = (y * map_w + x) * 3;
				idx = (y * 3 * map_w) + (x * 3);

				dx = (double)x / (double)map_w ;
				dy = (double)y / (double)map_h;
				//float val = (perlin.perlin2DConfigurable(x, y, seed, scale, octaves, lacunarity, persistence) * 0.5f + 0.5f) * 255.0f;
				val = (perlin.GetHeight(dx, dy) * 0.5f + 0.5f) * r;
				val = glm::clamp(val, 0.0, 255.0);

				for (auto& biome : biomes) {
					if (val >= biome.thr) {
						tileMap[idx] = biome.c1;
						tileMap[idx + 1] = biome.c2;
						tileMap[idx + 2] = biome.c3;
						break;
					}
				}
			}
		}
		//map = Texture2D{ tileMap, (int)map_w, (int)map_h , GL_RGB , GL_RGBA };
		//map.genMipmap();
		data = tileMap;
	}

	int map_w;
	int map_h;

private:
	struct Biome {
		uint8_t type;
		double thr;
		char c1;
		char c2;
		char c3;

		bool compare(const char c1, const char c2, const char c3) const {
			return (this->c1 == c1 && this->c2 == c2 && this->c3 == c3);
		}
	};

	void sortBiomes() {
		std::sort(biomes.begin(), biomes.end(), [](const Biome& a, const Biome& b) {
			return a.thr > b.thr;
			});
	}

	std::vector<Biome> biomes;

	Texture2D map;
	char* data;
};