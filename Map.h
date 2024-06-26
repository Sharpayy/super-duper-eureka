#pragma once
#include "Bezier.h"
#include "perlin.h"
#include <iostream>
#include <vector>
#include <algorithm>

typedef union _int64_2x32
{
	int64_t d64;
	int32_t d32[2];

} int64_2x32;

typedef struct _perlin_data_buffer
{
	int map_x;
	int map_y;
	double persistence;
	double frequency;
	double amplitude;
	int octaves;
	int randomseed;

	char pad[8];

} perlin_data_buffer;


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

class Map {
public:
	Map() = default;
	Map(float map_w, float map_h, double persistence, double frequency, double amplitude, uint8_t octaves, uint32_t randomseed, Program pp, bool CPU = true);

	void createMap();

	uint8_t* LoadTextureData();

	void ReleaseTextureData();

	Texture2D getMap();

	char* getData();

	uint8_t getBiomeTypeAdv(float x, float y, float scale);

	uint8_t getBiomeThr(float x, float y, float scale, int s = 3);

private:
	void generateMapData(double persistence, double frequency, double amplitude, uint8_t octaves, uint32_t randomseed);

	int map_w;
	int map_h;

private:
	struct Biome {
		uint8_t type;
		uint8_t thr;
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

	uint32_t ssboBiomes;
	bool CPU;

	Texture2D map;
	char* data;
};