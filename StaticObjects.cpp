#include "StaticObjects.h"

StaticObj::StaticObj() = default;

StaticObj::StaticObj(glm::fvec2 position, uint8_t type) {
	this->position = position;
	this->type = type;
}

uint8_t StaticObj::getType() {
	return this->type;
}