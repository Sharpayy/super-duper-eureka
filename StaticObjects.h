#pragma once

#include <glm.hpp>
#include "Renderable.h"

#define RENDER_MODEL_AIRPORT 12
#define RENDER_MODEL_TOWER 13

struct StaticObj : public Renderable {
	StaticObj();
	StaticObj(glm::fvec2 position, uint8_t type, uint8_t npm);
	glm::fvec2 position;

	uint8_t getType();
private:
	uint32_t npm;
	uint8_t type;
};