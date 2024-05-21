#include "Aircraft.h"

float AirCraft::CalcAngle()
{
	glm::vec2 b = path.getBezierPosition(path.GetCurrentSection(), 0.0f, false);
	glm::vec2 a = path.getBezierPosition(path.GetCurrentSection(), 0.00001f, false);
	glm::vec2 angle = normalize(b - a);
	return atan2f(angle.y, angle.x);
}

Glider::Glider(glm::fvec2 position, glm::fvec2 destination, uint8_t type) {
	this->position = position;
	path = { position, destination };

	this->type = type;
}

AirCraft::AirCraft(glm::fvec2 position, glm::fvec2 destination, uint8_t type) {
	this->position = position;
	path = { position, destination };
}

uint8_t AirCraft::getType() {
	return this->type;
}

Ballon::Ballon(glm::fvec2 position, glm::fvec2 destination, uint8_t type) {
	this->position = position;
	path = { position, destination };

	this->type = type;
}

Helicopter::Helicopter(glm::fvec2 position, glm::fvec2 destination, uint8_t type) {
	this->position = position;
	path = { position, destination };

	this->type = type;
}

Jet::Jet(glm::fvec2 position, glm::fvec2 destination, uint8_t type) {
	this->position = position;
	path = { position, destination };

	this->type = type;
}

Plane::Plane(glm::fvec2 position, glm::fvec2 destination, uint8_t type) {
	this->position = position;
	path = { position, destination };

	this->type = type;
}

void AirCraft::onUpdate() {

}

void AirCraft::SetAngle(float a)
{
	angle = a;
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

glm::fvec2 FlyPath::getBezierPosition(BezierCurveParametersA* param, float dt, bool change) {

	glm::vec2 p0 = param->str_pos;
	glm::vec2 p1 = param->mid0_pos;
	glm::vec2 p2 = param->mid1_pos;
	glm::vec2 p3 = param->end_pos;

	this->t += dt;

	glm::fvec2 Point = (1 - t) * (1 - t) * (1 - t) * p0 + 3 * (1 - t) * (1 - t) * t * p1 + 3 * (1 - t) * t * t * p2 + t * t * t * p3;

	if (!change)
		this->t -= dt;

	if (t >= 1.0f)
	{
		t = 0.0f;
		currentPathSection++;
	}

	return Point;
}

float FlyPath::GetCurrentSectionDistance()
{
	return t;
}

BezierCurveParametersA* FlyPath::getData() {
	return path.data();
}

BezierCurveParametersA* FlyPath::GetCurrentSection()
{
	return (BezierCurveParametersA*)&path.at(currentPathSection);
}

uint32_t FlyPath::FetchRenderInfo(BezierCurveParametersA* data, uint32_t max)
{
	memcpy(data, path.data(), sizeof(BezierCurveParametersA) * path.size());
	return path.size();
}

void FlyPath::AddPoint(glm::vec2 p)
{
	path.at(path.size() - 1).end_pos = p;
	path.push_back(FlyPathPoint(p, destination));
	UpdatePath();
	//ValidateAngles(path.size() - 2);
}

void FlyPath::ValidateAngles(uint32_t idx)
{
	if (idx == path.size())
		return;

	FlyPathPoint p0 = path.at(idx);
	FlyPathPoint p1 = path.at(idx+1);

	float len = 80.0f;
	float a = glm::dot(normalize(p0.str_pos), normalize(p1.end_pos));

	float bad_angle = cos(glm::radians(55.0f));

	glm::vec2 add_pos0 = normalize(p0.mid1_pos) * len;
	glm::vec2 add_pos1 = normalize(p1.mid0_pos) * len;

	path.push_back(FlyPathPoint(add_pos1, destination));
	(&(path.at(idx)))->end_pos = add_pos0;
	(&(path.at(idx + 1)))->str_pos = add_pos0;
	(&(path.at(idx + 1)))->end_pos = add_pos1;

	UpdatePath();

	// duże dodatnie = zle
	printf("%f\n", a);
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
