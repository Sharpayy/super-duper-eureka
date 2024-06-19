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
	Map(float map_w, float map_h, double persistence, double frequency, double amplitude, uint8_t octaves, uint32_t randomseed, Program pp, bool CPU = true)
	{
		this->map_w = map_w;
		this->map_h = map_h;

		biomes = {
			Biome{ MOUNTAIN, 191, (char)139, (char)137, (char)137 },
			Biome{ HILL, 155, (char)100, (char)100, (char)100 },
			Biome{ GRASSLAND, 76, (char)34, (char)139, (char)34 },
			Biome{ BEACH, 38, (char)238, (char)214, (char)175 },
			Biome{ WATER, 0, (char)0, (char)105, (char)148 },
			Biome{ DESERT, 51, (char)237, (char)201, (char)175 },
			Biome{ SNOW, 200, (char)255, (char)250, (char)250 },
			Biome{ SWAMP, 25, (char)47, (char)79, (char)79 },
			Biome{ TUNDRA, 125, (char)0, (char)90, (char)0 },
			Biome{ SAVANNA, 100, (char)0, (char)110, (char)0 },
		};

		this->CPU = CPU;
		if (!CPU) {
			data = new char[(int)map_w * (int)map_h];
			glGenBuffers(1, &ssboBiomes);
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboBiomes);
			glBufferData(GL_SHADER_STORAGE_BUFFER, (int)map_w * (int)map_h * 4, NULL, GL_STATIC_READ);

			perlin_data_buffer perlinData;

			memset(&perlinData, 1, sizeof(perlin_data_buffer));

			perlinData.map_x = (int)map_w;
			perlinData.map_y = (int)map_h;
			perlinData.octaves = octaves;
			perlinData.persistence = persistence;
			perlinData.frequency = frequency;
			perlinData.amplitude = amplitude;
			perlinData.randomseed = randomseed;

			uint32_t uboPerlinData;
			glGenBuffers(1, &uboPerlinData);
			glBindBuffer(GL_UNIFORM_BUFFER, uboPerlinData);
			glBufferData(GL_UNIFORM_BUFFER, sizeof(perlin_data_buffer), &perlinData, GL_STATIC_DRAW);

			map = Texture2D(NULL, (int)map_w, (int)map_h, GL_RGBA, GL_RGBA8);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

			glUseProgram(pp.id);
			map.bind();
			glBindImageTexture(2, map.id, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA8);
			glBindBuffer(GL_UNIFORM_BUFFER, uboPerlinData);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, ssboBiomes);
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboBiomes);
			glBindBufferBase(GL_UNIFORM_BUFFER, 1, uboPerlinData);

			printf("call!");
			glDispatchCompute((int)map_w / 10, (int)map_h / 10, 1);
			glMemoryBarrier(GL_ALL_BARRIER_BITS);
			map.bind();
			map.genMipmap();

			glDeleteBuffers(1, &uboPerlinData);
			LoadTextureData();
		}
		else {
			sortBiomes();
			generateMapData(persistence, frequency, amplitude, octaves, randomseed);
			createMap();
		}
	}

	void createMap() {
		map = Texture2D{ data, (int)map_w, (int)map_h , GL_RGB , GL_RGBA };
		map.genMipmap();
	}

	uint8_t* LoadTextureData()
	{
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboBiomes);
		data = (char*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
		return (uint8_t*)data;
	}

	void ReleaseTextureData()
	{
		if (CPU) delete data;
		else {
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboBiomes);
			glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
			glDeleteBuffers(1, &ssboBiomes);
		}
		data = nullptr;
	}



	Texture2D getMap() {
		return map;
	}

	char* getData() {
		return data;
	}

	uint8_t getBiomeTypeAdv(float x, float y, float scale)
	{
		float true_x = (x * scale) + (map_w / 2.0f);
		float true_y = (y * scale) + (map_h / 2.0f);
		int idx = ((int)true_y * map_w) + ((int)true_x);
		char* data = getData();

		if (!CPU) {
			return (uint8_t)((((int*)(data))[idx]) >> 16);
		}
		char c1, c2, c3;
		idx = ((int)true_y * 3 * map_w) + ((int)true_x * 3);
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

	uint8_t getBiomeThr(float x, float y, float scale, int s = 3) {
		float true_x = (x * scale) + (map_w / 2.0f);
		float true_y = (y * scale) + (map_h / 2.0f);
		int idx = ((int)true_y * map_w) + ((int)true_x);
		char* data = getData();

		if (!CPU) {
			int thr = ((int*)data)[idx] & 0x0000FFFF;
			return thr;
		}
		char c1, c2, c3;
		idx = ((int)true_y * 3 * map_w) + ((int)true_x * 3);
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

private:
	void generateMapData(double persistence, double frequency, double amplitude, uint8_t octaves, uint32_t randomseed) {
		int size = map_w * map_h;
		char* tileMap = new char[size * 3];

		PerlinNoiseGenerator perlin{ persistence, frequency, amplitude, octaves, randomseed, };
		double dx, dy;
		int idx;
		double val;
		double r = 255.0;
		for (int y = 0; y < map_h; y++) {
			for (int x = 0; x < map_w; x++) {
				//idx = (y * map_w + x) * 3;
				idx = (y * 3 * map_w) + (x * 3);

				dx = (double)x / (double)map_w;
				dy = (double)y / (double)map_h;
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
		data = tileMap;
	}

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