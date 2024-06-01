#pragma once

struct Map {
	Map() = default;
	int sizeX;
	int sizeY;

	int scale;

	float Water;
	float Ground;
	float Mountains;
};

struct Config {

//#define MAP_WIDTH 10000
//#define MAP_HEIGHT 10000
//#define SCALE 10
//#define MAP_OFFSETX 0
//#define MAP_OFFSETY 0
//#define N_AIRPORTS 1
//#define N_TOWERS 200
//#define N_AIRCRAFTS 500

	int n_airports;
	int n_towers;
	int n_aircrafts;

	Map p

};