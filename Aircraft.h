#pragma once
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <queue>
#include <vector>
#include "Renderable.h"

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
	
	FlyPath() = default;
	FlyPath(glm::vec2 start, glm::vec2 end);

	uint32_t FetchRenderInfo(BezierCurveParametersA* data, uint32_t max);
	uint32_t GetPointAmount();

	void ChangeDestinatination(glm::vec2 dest);
	void AddPoint(glm::vec2 p);
	void ValidateAngles(uint32_t idx);

	BezierCurveParametersA* getData();
	BezierCurveParametersA* GetCurrentSection();

	glm::fvec2 getBezierPosition(BezierCurveParametersA* param, float dt, bool change = true);
	float GetCurrentSectionDistance();

private:
	float t;
	void UpdatePath();
};

#define RENDER_MODEL_BALLON 1
#define RENDER_MODEL_JET 2
#define RENDER_MODEL_HELICOPTER 3
#define RENDER_MODEL_GLIDER 4
#define RENDER_MODEL_PLANE 5

class AirCraft : public Renderable {
public:
	AirCraft() = default;
	AirCraft(glm::fvec2 position, glm::fvec2 destination, uint8_t type);

	void onUpdate();

	glm::fvec2 position;
	FlyPath path;
	float distanceToGround;
	float speed;
	float acceleration;
	float angle;

	uint8_t getType();
	float CalcAngle();
	void SetAngle(float a);

protected:
	uint8_t type;
};

class Ballon : public AirCraft {
public:
	Ballon() = default;
	Ballon(glm::fvec2 position, glm::fvec2 destination, uint8_t type);
};

class Jet : public AirCraft {
public:
	Jet() = default;
	Jet(glm::fvec2 position, glm::fvec2 destination, uint8_t type);
};

class Helicopter : public AirCraft {
public:
	Helicopter() = default;
	Helicopter(glm::fvec2 position, glm::fvec2 destination, uint8_t type);
};

class Glider : public AirCraft {
public:
	Glider() = default;
	Glider(glm::fvec2 position, glm::fvec2 destination, uint8_t type);
};

class Plane : public AirCraft {
public:
	Plane() = default;
	Plane(glm::fvec2 position, glm::fvec2 destination, uint8_t type);
};