#pragma once
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <queue>
#include <vector>
#include <unordered_map>
#include <string>
#include "Renderable.h"
#include "StaticObjects.h"
#include <string>

#include <glew.h>
#define GLT_IMPLEMENTATION
#define GLT_MANUAL_VIEWPORT
#include "glText.h"

class AircraftRenderData
{
public:
	AircraftRenderData()
	{
		model = gltCreateText();
		brand = gltCreateText();
		speed = gltCreateText();
		height = gltCreateText();

		scale = glm::vec2(200.0f, 50.0f);
		posit = glm::vec2(0.0f);
		f = 0.0f;
	}

	float GetFullHeight()
	{
		return gltGetTextHeight(model, scale.y) + gltGetTextHeight(brand, scale.y) + gltGetTextHeight(speed, scale.y) + gltGetTextHeight(height, scale.y);
	}
	
	void Render(glm::mat4 _mvp)
	{
		gltBeginDraw();

		float w = gltGetTextWidth(model, scale.x);
		float h = gltGetTextHeight(model, scale.y);
		glm::vec2 offset = glm::vec2(0.0f);
		
		glm::mat4 m = glm::mat4(1.0f);
		m = glm::translate(m, glm::vec3(posit, 2.0f));
		m = glm::translate(m, glm::vec3(w / 2.0f, h / 2.0f, 2.0f));
		m = glm::rotate(m, glm::radians(180.0f), glm::vec3(1, 0, 0));
		m = glm::translate(m, glm::vec3(-w / 2.0f, -h / 2.0f, 0.0f));
		m = glm::scale(m, glm::vec3(scale, 0.0f));

		gltDrawText(model, &(_mvp * m)[0].x);
		offset.y -= h;


		 w = gltGetTextWidth(brand, scale.x);
		 h = gltGetTextHeight(brand, scale.y);

		m = glm::mat4(1.0f);
		m = glm::translate(m, glm::vec3(posit + offset, 2.0f));
		m = glm::translate(m, glm::vec3(w / 2.0f, h / 2.0f, 2.0f));
		m = glm::rotate(m, glm::radians(180.0f), glm::vec3(1, 0, 0));
		m = glm::translate(m, glm::vec3(-w / 2.0f, -h / 2.0f, 0.0f));
		m = glm::scale(m, glm::vec3(scale, 0.0f));

		gltDrawText(brand, &(_mvp * m)[0].x);
		offset.y -= h;

		w = gltGetTextWidth(speed, scale.x);
		h = gltGetTextHeight(speed, scale.y);

		m = glm::mat4(1.0f);
		m = glm::translate(m, glm::vec3(posit + offset, 2.0f));
		m = glm::translate(m, glm::vec3(w / 2.0f, h / 2.0f, 2.0f));
		m = glm::rotate(m, glm::radians(180.0f), glm::vec3(1, 0, 0));
		m = glm::translate(m, glm::vec3(-w / 2.0f, -h / 2.0f, 0.0f));
		m = glm::scale(m, glm::vec3(scale, 0.0f));

		gltDrawText(speed, &(_mvp * m)[0].x);
		offset.y -= h;

		w = gltGetTextWidth(height, scale.x);
		h = gltGetTextHeight(height, scale.y);

		m = glm::mat4(1.0f);
		m = glm::translate(m, glm::vec3(posit + offset, 2.0f));
		m = glm::translate(m, glm::vec3(w / 2.0f, h / 2.0f, 2.0f));
		m = glm::rotate(m, glm::radians(180.0f), glm::vec3(1, 0, 0));
		m = glm::translate(m, glm::vec3(-w / 2.0f, -h / 2.0f, 0.0f));
		m = glm::scale(m, glm::vec3(scale, 0.0f));

		gltDrawText(height, &(_mvp * m)[0].x);
		offset.y -= h;

		gltEndDraw();
	}

	void SetModel(const char* v)
	{
		std::string s = std::string("Model: ");
		s.append(v);
		gltSetText(model, s.c_str());
	}

	void SetColor(float r, float g, float b)
	{
		gltColor(r, g, b, 0.0f);
	}

	void SetBrand(const char* v)
	{
		std::string s = std::string("Brand: ");
		s.append(v);
		gltSetText(brand, s.c_str());
	}

	void SetSpeed(float v)
	{
		std::string s = std::string("Speed: ");
		s.append(std::to_string(v));
		gltSetText(speed, s.c_str());
	}

	void SetHeigth(float v)
	{
		std::string s = std::string("Height: ");
		s.append(std::to_string(v));
		gltSetText(height, s.c_str());
	}

	void SetPosition(glm::fvec2 position) {
		this->posit = position;
	}

	void SetScale(glm::fvec2 scale) {
		this->scale = scale;
	}

private:
	GLTtext* model;
	GLTtext* brand;
	GLTtext* speed;
	GLTtext* height;

	glm::vec2 scale;
	glm::vec2 posit;

	float f;
};

typedef struct BezierCurveParametersA
{
	glm::vec2 str_pos;
	glm::vec2 mid0_pos;
	glm::vec2 mid1_pos;
	glm::vec2 end_pos;
};

class FlyPathPoint : public BezierCurveParametersA
{
public:
	glm::vec2 GetPoint();
	FlyPathPoint() = default;
	FlyPathPoint(glm::vec2 p);
	FlyPathPoint(glm::vec2 s, glm::vec2 d);
};

typedef unsigned long long uint64_t;

class FlyPath
{
public:
	std::vector<FlyPathPoint> path;
	uint32_t currentPathSection;
	glm::vec2 destination;
	glm::vec2 start;
	
	FlyPath() = default;
	FlyPath(glm::vec2 start, glm::vec2 end);

	uint32_t FetchRenderInfo(BezierCurveParametersA* data, uint32_t max);
	uint32_t GetPointAmount();

	void ChangeDestinatination(glm::vec2 dest);
	void AddPoint(glm::vec2 p);
	void ValidateAngles(uint32_t idx);
	void ValidateAnglesNew();
	void UpdateAngles(uint32_t idx);
	float BezierSingleLength(BezierCurveParametersA* data);
	BezierCurveParametersA* getData();
	BezierCurveParametersA* GetCurrentSection();

	glm::fvec2 getBezierPosition2D(BezierCurveParametersA* param, float dt, bool change = true);
	float getBezierPosition1D(BezierCurveParametersA* param, float dt);
	
	float GetCurrentSectionDistance();
	void resetT() { t = 0; };

private:
	float t;
	float ht;
	void UpdatePath(float len = 120.0f);
};

#define RENDER_MODEL_BALLOON 1
#define RENDER_MODEL_JET 2
#define RENDER_MODEL_HELICOPTER 3
#define RENDER_MODEL_GLIDER 4
#define RENDER_MODEL_PLANE 5

#define minNPM 9448.8f
#define maxNPM 12801.6f

class AirCraft : public Renderable {
public:
	AirCraft();
	//AirCraft(glm::fvec2 position, glm::fvec2 destination, uint8_t type);

	void onUpdate();

	glm::fvec2 position;
	FlyPath path;

	float distanceToGround;
	float speed;
	float acceleration;
	float angle;

	float dist;
	bool collide = false;

	uint8_t getType() const;
	std::string GetName() const;
	float CalcAngle();
	void SetAngle(float a);
	BezierCurveParametersA* heightData;

protected:
	std::string name;
	uint8_t type;
};

class Ballon : public AirCraft {
public:
	Ballon() = default;
	Ballon(glm::fvec2 position, float baseHeight, StaticObj* destination, uint8_t type);
};

class Jet : public AirCraft {
public:
	Jet() = default;
	Jet(glm::fvec2 position, float baseHeight, StaticObj* destination, uint8_t type);
};

class Helicopter : public AirCraft {
public:
	Helicopter() = default;
	Helicopter(glm::fvec2 position, float baseHeight, StaticObj* destination, uint8_t type);
};

class Glider : public AirCraft {
public:
	Glider() = default;
	Glider(glm::fvec2 position, float baseHeight, StaticObj* destination, uint8_t type);
};

class Plane : public AirCraft {
public:
	Plane() = default;
	Plane(glm::fvec2 position, float baseHeight, StaticObj* destination, uint8_t type);
};

//MISC
float normalizeVal(float value, float minValue, float maxValue);

float mapValueToRange(float normalizedValue, float minValue, float maxValue);