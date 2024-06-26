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
	AircraftRenderData();

	void Render(glm::mat4 _mvp);

	void SetModel(const char* v);

	void SetColor(float r, float g, float b);

	void SetBrand(const char* v);

	void SetSpeed(float v);

	void SetHeigth(float v);

	void SetPosition(glm::fvec2 position);

	void SetScale(glm::fvec2 scale);

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

#define RENDER_MODEL_BALLOON 0
#define RENDER_MODEL_JET 1
#define RENDER_MODEL_HELICOPTER 2
#define RENDER_MODEL_GLIDER 3
#define RENDER_MODEL_PLANE 4

struct acHeightData {
	float stMinH;
	float stMaxH;
	float acMinH;
	float acMaxH;
};

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
	Ballon(glm::fvec2 position, float baseHeight, StaticObj* destination, uint8_t type, acHeightData* data);
};

class Jet : public AirCraft {
public:
	Jet() = default;
	Jet(glm::fvec2 position, float baseHeight, StaticObj* destination, uint8_t type, acHeightData* data);
};

class Helicopter : public AirCraft {
public:
	Helicopter() = default;
	Helicopter(glm::fvec2 position, float baseHeight, StaticObj* destination, uint8_t type, acHeightData* data);
};

class Glider : public AirCraft {
public:
	Glider() = default;
	Glider(glm::fvec2 position, float baseHeight, StaticObj* destination, uint8_t type, acHeightData* data);
};

class Plane : public AirCraft {
public:
	Plane() = default;
	Plane(glm::fvec2 position, float baseHeight, StaticObj* destination, uint8_t type, acHeightData* data);
};

//MISC
float normalizeVal(float value, float minValue, float maxValue);

float mapValueToRange(float normalizedValue, float minValue, float maxValue);