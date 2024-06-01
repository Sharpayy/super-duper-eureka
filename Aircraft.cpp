#include "Aircraft.h"

float AirCraft::CalcAngle()
{
	glm::vec2 b = path.getBezierPosition(path.GetCurrentSection(), 0.0f, false);
	glm::vec2 a = path.getBezierPosition(path.GetCurrentSection(), 0.00001f, false);
	glm::vec2 angle = normalize(b - a);
	return atan2f(angle.y, angle.x);
}

AirCraft::AirCraft(glm::fvec2 position, glm::fvec2 destination, uint8_t type) {
	this->position = position;
	path = { position, destination };
	//collisionDist = 0.0f;
	dist = 0;
}

Glider::Glider(glm::fvec2 position, glm::fvec2 destination, uint8_t type) {
	this->position = position;
	path = { position, destination };

	this->type = type;
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

	if (t >= 1.0f && change == true)
	{
		assert(currentPathSection < path.size());
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
	ValidateAnglesNew();
}

float Ang(glm::vec2 a, glm::vec2 b)
{
	return atan2(a.x * b.y - a.y * b.x, a.x * b.x + a.y * b.y);
}

void FlyPath::ValidateAngles(uint32_t idx)
{
	uint32_t Last = idx - 1;
	uint32_t PreLast = idx - 2;

	FlyPathPoint p1 = path.at(Last);
	FlyPathPoint p0 = path.at(PreLast);

	float len = 40.0f;
	float a = glm::degrees(Ang(glm::normalize(p0.end_pos - p0.str_pos), glm::normalize(p0.end_pos - p1.end_pos)));

	float bad_angle = 60.0f;

	printf("cos: %f\n", a);
	if (abs(a) > bad_angle)
		return;

	printf("Correction!\n");
	float a_ = (p1.end_pos.y - p1.str_pos.y) / (p1.end_pos.x - p1.str_pos.x);
	float b_ = -(a_ * p1.str_pos.x) - p1.str_pos.y;

	glm::vec2 sp = glm::vec2((p1.end_pos.x + p1.str_pos.x) / 2.0f, (p1.end_pos.y + p1.str_pos.y) / 2.0f);

	float a1 = -1.0f / a_;
	float b1 = -(a1 * sp.x) - sp.y;

	glm::vec2 pl = glm::normalize(p1.str_pos);

	float side = 1.0f;
	float l = 0.0f;

	glm::vec2 dir1 = normalize(glm::vec2(1.0f, a1) * side);
	glm::vec2 p = dir1 * len;
	path.at(idx - 1).end_pos = p + sp;
	path.push_back(FlyPathPoint(p + sp, destination));

	UpdatePath(l);
}

void FlyPath::ValidateAnglesNew()
{
	uint32_t Last = path.size() - 1;
	uint32_t PreLast = path.size() - 2;

	FlyPathPoint p1 = path.at(Last);
	FlyPathPoint p0 = path.at(PreLast);

	float len = 40.0f;
	float a = glm::degrees(Ang(glm::normalize(p0.end_pos - p0.str_pos), glm::normalize(p0.end_pos - p1.end_pos)));

	float bad_angle = 60.0f;

	printf("cos: %f\n", a);
	if (abs(a) > bad_angle)
		return;

	printf("Correction!\n");
	float a_ = (p1.end_pos.y - p1.str_pos.y) / (p1.end_pos.x - p1.str_pos.x);
	float b_ = -(a_ * p1.str_pos.x) - p1.str_pos.y;

	glm::vec2 sp = glm::vec2((p1.end_pos.x + p1.str_pos.x) / 2.0f, (p1.end_pos.y + p1.str_pos.y) / 2.0f);

	float a1 = -1.0f / a_;
	float b1 = -(a1 * sp.x) - sp.y;

	glm::vec2 pl = glm::normalize(p1.str_pos);

	//float side = -1.0f + (2.0f * (pl.x < 0.0f && pl.y < 0.0f));
	float side = 1.0f;
	float l = 80.0f;

	glm::vec2 dir1 = normalize(glm::vec2(1.0f, a1) * side);
	if (GetCurrentSection()->str_pos.x > destination.x) {
		dir1.x *= 1;
		sp.x *= 1;
	}
	else {
		dir1.x *= -1;
		sp.x *= -1;
	}
	if (abs(GetCurrentSection()->str_pos.y) > abs(destination.y)) {
		dir1.y *= 1;
		sp.y *= -1;
	}
	else {
		dir1.y *= -1;
		sp.y *= 1;
	}
	glm::vec2 p = dir1 * len;
	path.at(path.size() - 1).end_pos = p + sp;

	path.push_back(FlyPathPoint(p + sp, destination));

	UpdatePath(l);
}

void FlyPath::UpdateAngles(uint32_t idx)
{
}

void FlyPath::UpdatePath(float len)
{
	FlyPathPoint* fpp;
	FlyPathPoint* nfpp;

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

/*
	uint32_t Last = path.size() - 1;
	uint32_t PreLast = path.size() - 2;

	FlyPathPoint p0 = path.at(Last);
	FlyPathPoint p1 = path.at(PreLast);

	float len = 10.0f;
	float a = glm::dot(normalize(p0.end_pos), normalize(p1.str_pos));

	float bad_angle = cos(glm::radians(55.0f));

	if (a < bad_angle)
		return;

	glm::vec2 add_pos1 = normalize(p0.mid1_pos) * len;
	glm::vec2 add_pos0 = normalize(p1.mid0_pos) * len;
	//glm::vec2 add_pos0 = p1.str_pos;

	path.at(PreLast).end_pos = add_pos0;
	path.at(Last).end_pos = add_pos1;
	path.at(Last).str_pos = add_pos0;
	path.push_back(FlyPathPoint(add_pos1, destination));

	UpdatePath();
*/
