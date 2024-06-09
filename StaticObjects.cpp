#include "StaticObjects.h"

StaticObj::StaticObj() = default;

StaticObj::StaticObj(glm::fvec2 position, uint8_t type, uint8_t npm) {
	this->position = position;
	this->type = type;
	this->npm = npm;
}

uint8_t StaticObj::getType() {
	return this->type;
}