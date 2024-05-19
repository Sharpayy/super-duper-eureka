#pragma once

#include <glm.hpp>
#include "Renderable.h"

#define RENDER_MODEL_AIRPORT 12
#define RENDER_MODEL_TOWER 13

struct StaticObj : public Renderable {
	StaticObj();
	StaticObj(glm::fvec2 position, uint8_t type);
	glm::fvec2 position;

	uint8_t getType();
private:
	uint8_t type;
};