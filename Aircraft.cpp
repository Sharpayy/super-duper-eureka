#include "Aircraft.h"

AirCraft::AirCraft(glm::fvec2 position) {
	this->position = position;
}

Ballon::Ballon(glm::fvec2 position, glm::fvec2 destination) {
	this->position = position;
	this->destination = destination;
}

Helicopter::Helicopter(glm::fvec2 position, glm::fvec2 destination) {
	this->position = position;
	this->destination = destination;
}

Jet::Jet(glm::fvec2 position, glm::fvec2 destination) {
	this->position = position;
	this->destination = destination;
}

Plane::Plane(glm::fvec2 position, glm::fvec2 destination) {
	this->position = position;
	this->destination = destination;
}

Glider::Glider(glm::fvec2 position, glm::fvec2 destination) {
	this->position = position;
	this->destination = destination;
}

void AirCraft::onUpdate() {

}
