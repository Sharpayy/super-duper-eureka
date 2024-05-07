#pragma once
#include <glm.hpp>

typedef unsigned long long uint64_t;

class Renderable
{
public:
	uint64_t LongId;
};

#define RENDER_MODEL_BALLON 1
#define RENDER_MODEL_JET 2
#define RENDER_MODEL_HELICOPTER 3
#define RENDER_MODEL_GLIDER 4
#define RENDER_MODEL_PLANE 5

class AirCraft : Renderable {
public:
	AirCraft() = default;
	AirCraft(glm::fvec2 position);

	void onUpdate();

	float distanceToGround;
	glm::fvec3 position;
	float speed;
	float acceleration;
	float angle;
};

class Ballon : AirCraft {};
class Jet : AirCraft {};
class Helicopter : AirCraft {};
class Glider : AirCraft {};
class Plane : AirCraft {};