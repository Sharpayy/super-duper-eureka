#pragma once
//
#include <glm.hpp>
#include "Renderable.h"

struct StaticObj : Renderable {
	StaticObj() = default;
	StaticObj(glm::fvec2 position);
	glm::fvec2 position;
};