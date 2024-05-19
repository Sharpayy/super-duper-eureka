#include "Aircraft.h"

AirCraft::AirCraft(glm::fvec2 position, glm::fvec2 destination) {
	this->position = position;
	path = { position, destination };
}

uint8_t AirCraft::getType() {
	return this->type;
}

Ballon::Ballon(glm::fvec2 position, glm::fvec2 destination) {
	this->position = position;
	path = { position, destination };
	this->type = RENDER_MODEL_BALLON;
}

Helicopter::Helicopter(glm::fvec2 position, glm::fvec2 destination) {
	this->position = position;
	path = { position, destination };
	this->type = RENDER_MODEL_JET;
}

Jet::Jet(glm::fvec2 position, glm::fvec2 destination) {
	this->position = position;
	path = { position, destination };

	this->type = RENDER_MODEL_JET;
}

Plane::Plane(glm::fvec2 position, glm::fvec2 destination) {
	this->position = position;
	path = { position, destination };

	this->type = RENDER_MODEL_PLANE;
}

Glider::Glider(glm::fvec2 position, glm::fvec2 destination) {
	this->position = position;
	path = { position, destination };

	this->type = RENDER_MODEL_GLIDER;
}

void AirCraft::onUpdate() {

}

glm::vec2 FlyPathPoint::GetPoint()
{
	return str_pos;
}

FlyPathPoint::FlyPathPoint(glm::vec2 p)
{
	str_pos = p;
}

FlyPathPoint::FlyPathPoint(glm::vec2 s, glm::vec2 d)
{
	str_pos = s;
	end_pos = d;
}

FlyPath::FlyPath(glm::vec2 start, glm::vec2 end)
{
	t = 0.0f;
	currentPathSection = 0;
	path = std::vector<FlyPathPoint>();
	path.push_back(FlyPathPoint(start, end));
	destination = end;

	UpdatePath();
}

glm::fvec2 FlyPath::getBezierPosition(BezierCurveParametersA* param, float dt) {

	glm::vec2 p0 = param->str_pos;
	glm::vec2 p1 = param->mid0_pos;
	glm::vec2 p2 = param->mid1_pos;
	glm::vec2 p3 = param->end_pos;
	
	this->t += dt;
	glm::fvec2 Point = (1 - t) * (1 - t) * (1 - t) * p0 + 3 * (1 - t) * (1 - t) * t * p1 + 3 * (1 - t) * t * t * p2 + t * t * t * p3;

	return Point;
}

BezierCurveParametersA* FlyPath::getData() {
	return path.data();
}

uint32_t FlyPath::FetchRenderInfo(BezierCurveParametersA* data, uint32_t max)
{
	memcpy(data, path.data(), sizeof(BezierCurveParametersA) * path.size());
	return path.size();
}

void FlyPath::AddPoint(glm::vec2 p)
{
	path.at(path.size() - 1).end_pos = p;
	//path.end()->end_pos = p;
	path.push_back(FlyPathPoint(p, destination));
	UpdatePath();
}

void FlyPath::UpdatePath()
{
	FlyPathPoint* fpp;
	FlyPathPoint* nfpp;

	float len = 120.0f;

	for (uint32_t i = 0; i != path.size(); i++)
	{
		fpp = path.data() + i;

		glm::vec2 a = glm::normalize(fpp->str_pos - fpp->end_pos);
		fpp->mid0_pos = fpp->str_pos + a * -len;

	}

	for (uint32_t i = 0; i != path.size() - 1; i++)
	{
		fpp = path.data() + i;
		nfpp = path.data() + i + 1;

		glm::vec2 a = glm::normalize(nfpp->str_pos - nfpp->end_pos);
		fpp->mid1_pos = fpp->end_pos - a * -len;
	}

	path.at(path.size() - 1).mid1_pos = path.at(path.size() - 1).end_pos;

}
