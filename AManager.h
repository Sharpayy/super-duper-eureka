#pragma once
//Renderer
#include <cstdio>
#include "iWinSDL.h"
#include "iRendered.h"
#include "iReaders.h"
#include <ctime>
#include "Models.h"
#include <gtc/matrix_transform.hpp>
#include <functional>
#include "Objects.h"
#include <stb_image.h>

//Additional
#include "perlin.h"

#include "Aircraft.h"
#include <utility>

#ifndef _DEBUG
#define AM_ASSERT(A) assert(A)
#endif

class AManager {
public:
	AManager() = default;
	AManager(RenderGL& r) {
		this->r = r;
	}

	void onUpdate() {
		
	}

private:
	std::pair<glm::fvec2, glm::fvec2> generateRandomPath(int mapWidth, float mapHeight) {
		glm::fvec2 start, end;
		int x, y;
		x = ((rand() / INT_MAX) + 1) * mapWidth;
		y = ((rand() / INT_MAX) + 1) * mapHeight;
		start = { x, y };
		x = ((rand() / INT_MAX) + 1) * mapWidth;
		y = ((rand() / INT_MAX) + 1) * mapHeight;
		end = { x, y };
		return { start, end };
	}

	AirCraft* generateRandomAirCraft(int idx) {
		switch (idx)
		{
		case 0:
			return (AirCraft*)new Ballon{};
		case 1:
			return (AirCraft*)new Jet{};
		case 2:
			return (AirCraft*)new Helicopter{};
		case 3:
			return (AirCraft*)new Plane{};
		case 4:
			return (AirCraft*)new Glider{};
		default:
			assert("GenerateRandomAirCraft idx out of bound");
			break;
		}
	}

	void addModel(Texture2D texture, uint8_t idModel) {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		texture.genMipmap();
		//r.newModel(idModel, Square, simpleProgram, 6, GL_TRIANGLES, texture, 50);
	}

	void rotateAirCraft(AirCraft* airCraft, glm::fvec3 destination) {
		//FOR NOW
		//float angle = 0;

		//airCraft->angle;
	}
private:
	std::vector<AirCraft*> airCraftVec;

	RenderGL r;
};