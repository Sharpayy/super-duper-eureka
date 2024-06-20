#pragma once
#include <filesystem>
#define YAML_CPP_DLL
#define YAML_CPP_STATIC_DEFINE
#include <yaml-cpp/yaml.h>
#include <fstream>
#include <string>

struct Config {
	float n_airports = 1.0f;
	float n_towers = 1.0f;
	float n_aircrafts = 1.0f;

	//Map
	float map_width = 5000;
	float map_height = 5000;
	float scale = 1.0f;
	bool CPU = true;
	double persistence = 1.0;
	double frequency = 2.5;
	double amplitude = 1.0;
	uint8_t octaves = 7;
	uint32_t randomseed = 21324;

	//Aircrafts Height
	float aircraftsMinNPM = 9448.1f;
	float aircraftsMaxNPM = 12801.6f;
	//StaticObjects Height
	float staticObjectsMinNPM = 67.23;
	float staticObjectsMaxNPM = 156.63;

	//Collision
	float collsiionDetectionDistance = 200.0f;
	float collisionResponseHeight = 50.0f;
	float airportsCollisionDistance = 300.0f;
	float towersCollisionDistance = 100.0f;
};

bool fileExist(std::string name, std::string ext);

void loadConfig(Config& cfg, std::string fileName);
void saveConfig(Config& cfg, std::string fileName);

void yamlSerialize(Config& cfg, std::string fileName);
void yamlDeserialize(Config& cfg, std::string fileName);