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
#include <random>

#ifdef _DEBUG
#define AM_ASSERT(A) assert(A)
#endif

extern glm::mat4 BaseIconScaleMatrix;

class AManager {
public:
	AManager() = default;
	AManager(RenderGL& r, VertexBuffer& vertexBuff, Program program) {
		this->r = r;
		this->vertexBuff = vertexBuff;
		this->program = program;

		int x, y, c;
		Texture2D MapTexture;
		uint8_t* MapTextureData;

		MapTextureData = (uint8_t*)LoadImageData("helicopter.png", 1, &c, &x, &y);
		MapTexture = Texture2D(MapTextureData, x, y, GL_RGBA, GL_RGBA, GL_TEXTURE0);
		addModel(MapTexture, RENDER_MODEL_HELICOPTER);

		MapTextureData = (uint8_t*)LoadImageData("glider.png", 1, &c, &x, &y);
		MapTexture = Texture2D(MapTextureData, x, y, GL_RGBA, GL_RGBA, GL_TEXTURE0);
		addModel(MapTexture, RENDER_MODEL_GLIDER);

		MapTextureData = (uint8_t*)LoadImageData("ballon.png", 1, &c, &x, &y);
		MapTexture = Texture2D(MapTextureData, x, y, GL_RGBA, GL_RGBA, GL_TEXTURE0);
		addModel(MapTexture, RENDER_MODEL_BALLON);

		MapTextureData = (uint8_t*)LoadImageData("jet.png", 1, &c, &x, &y);
		MapTexture = Texture2D(MapTextureData, x, y, GL_RGBA, GL_RGBA, GL_TEXTURE0);
		addModel(MapTexture, RENDER_MODEL_JET);

		MapTextureData = (uint8_t*)LoadImageData("plane.png", 1, &c, &x, &y);
		MapTexture = Texture2D(MapTextureData, x, y, GL_RGBA, GL_RGBA, GL_TEXTURE0);
		addModel(MapTexture, RENDER_MODEL_PLANE);

		for (int i = 0; i < 50; i++) {
			airCraftVec.push_back(generateRandomAirCraft(i % 5, 1000, 1000));
		}

		//86 400
		timeScale = 1.0f / 3600.0f;
	}

	void onUpdate() {
		r.RenderSelectedModel(RENDER_MODEL_HELICOPTER);
		r.RenderSelectedModel(RENDER_MODEL_GLIDER);
		r.RenderSelectedModel(RENDER_MODEL_BALLON);
		r.RenderSelectedModel(RENDER_MODEL_JET);
		r.RenderSelectedModel(RENDER_MODEL_PLANE);
	}

private:
	std::pair<glm::fvec2, glm::fvec2> generateRandomPath(int mapWidth, float mapHeight) {
		glm::fvec2 start, end;
		float x, y;

		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_int_distribution<> distrW(0, mapWidth);
		std::uniform_int_distribution<> distrH(0, mapHeight);

		x = distrW(gen);
		y = distrH(gen);
		start = { x, y };
		x = distrW(gen);
		y = distrH(gen);
		end = { x, y };
		return { start, end };
	}

	AirCraft* generateRandomAirCraft(int idx, int mapWidth, int mapHeight) {
		std::pair<glm::fvec2, glm::fvec2> s_e;

		s_e = generateRandomPath(mapHeight, mapHeight);

		Glider* glider;
		Jet* jet;
		Ballon* ballon;
		Helicopter* helicopter;
		Plane* plane;
		switch (idx)
		{
		case 0:
			ballon = new Ballon{ s_e.first, s_e.second };
			r.newObject(RENDER_MODEL_BALLON, glm::translate(glm::mat4(1.0f), glm::fvec3{ ballon->position.x, ballon->position.y, 0.05f }) * BaseIconScaleMatrix, &ballon->LongId);
			return (AirCraft*)ballon;
		case 1:
			jet = new Jet{ s_e.first, s_e.second };
			r.newObject(RENDER_MODEL_JET, glm::translate(glm::mat4(1.0f), glm::fvec3{jet->position.x, jet->position.y, 0.05f}) * BaseIconScaleMatrix, &jet->LongId);
			return (AirCraft*)jet;
		case 2:
			 helicopter = new Helicopter{ s_e.first, s_e.second };
			r.newObject(RENDER_MODEL_HELICOPTER, glm::translate(glm::mat4(1.0f), glm::fvec3{ helicopter->position.x, helicopter->position.y, 0.05f }) * BaseIconScaleMatrix, &helicopter->LongId);
			return (AirCraft*)helicopter;
		case 3:
			plane = new Plane{ s_e.first, s_e.second };
			r.newObject(RENDER_MODEL_PLANE, glm::translate(glm::mat4(1.0f), glm::fvec3{ plane->position.x, plane->position.y, 0.05f }) * BaseIconScaleMatrix, &plane->LongId);
			return (AirCraft*)plane;
		case 4:
			glider = new Glider{ s_e.first, s_e.second };
			r.newObject(RENDER_MODEL_GLIDER, glm::translate(glm::mat4(1.0f), glm::fvec3{ glider->position.x, glider->position.y, 0.05f }) * BaseIconScaleMatrix, &glider->LongId);
			return (AirCraft*)glider;

		default:
			AM_ASSERT("GenerateRandomAirCraft idx out of bound");
			break;
		}
	}

	void addModel(Texture2D texture, uint8_t idModel) {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		texture.genMipmap();
		r.newModel(idModel, vertexBuff, program, 6, GL_TRIANGLES, texture, 50);
	}

	void rotateAirCraft(AirCraft* airCraft, glm::fvec3 destination) {
		//FOR NOW
		//float angle = 0;

		//airCraft->angle;
	}
private:
	std::vector<AirCraft*> airCraftVec;

	RenderGL r;
	VertexBuffer vertexBuff;
	Program program;

	float timeScale;
};