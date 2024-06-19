#pragma once
#include <glm.hpp>
#include "Renderable.h"
#include "Config.h"

#define RENDER_MODEL_AIRPORT 12
#define RENDER_MODEL_TOWER 13
#define RENDER_MODEL_EXPLOSION 14
#define RENDER_MODEL_MAP 15

struct StaticObj : public Renderable {
	StaticObj();
	StaticObj(glm::fvec2 position, uint8_t type, uint8_t npm);

	glm::fvec2 getPosition();
	uint8_t getType();
	uint32_t getNpm();

private:
	glm::fvec2 position;
	uint32_t npm;
	uint8_t type;
};