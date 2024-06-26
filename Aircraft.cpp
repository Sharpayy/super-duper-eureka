#include "Aircraft.h"

std::unordered_map<uint8_t, std::vector<std::string>> aircraftNames = {
		{RENDER_MODEL_BALLOON, {"Montgolfier", "Zeppelin NT", "Goodyear Blimp"}},
		{RENDER_MODEL_JET, {"Boeing 747", "Airbus A320", "Cessna 172", "Concorde", "Gulfstream G650"}},
		{RENDER_MODEL_HELICOPTER, {"Lockheed Martin F-35 Lightning II", "McDonnell Douglas DC-10", "Bell 206", "Eurocopter AS350"}},
		{RENDER_MODEL_GLIDER, {"Schleicher ASK 21", "Grob G102 Astir", "Schempp-Hirth Discus"}},
		{RENDER_MODEL_PLANE, {"Piper PA-28 Cherokee", "Beechcraft King Air", "Antonov An-225 Mriya", "Bombardier Global 7500", "Sukhoi Superjet 100"}}
};

float AirCraft::CalcAngle()
{
	glm::vec2 b = path.getBezierPosition2D(path.GetCurrentSection(), 0.0f, false);
	glm::vec2 a = path.getBezierPosition2D(path.GetCurrentSection(), 0.00001f, false);
	glm::vec2 angle = normalize(b - a);
	return atan2f(angle.y, angle.x);
}

AirCraft::AirCraft() {
	this->speed = 1.0f;
}



//AirCraft::AirCraft(glm::fvec2 position, glm::fvec2 destination, uint8_t type) {
//	this->position = position;
//	path = { position, destination };
//	//collisionDist = 0.0f;
//
//	dist = 0;
//}

std::string AirCraft::GetName() const {
	return this->name;
}

uint8_t AirCraft::getType() const {
	return this->type;
}

Glider::Glider(glm::fvec2 position, float baseHeight, StaticObj* destination, uint8_t type, acHeightData* data) {
	this->position = position;
	path = { position, destination->getPosition() };

	heightData = new BezierCurveParametersA{};
	heightData->str_pos = { 0, baseHeight };
	heightData->mid0_pos = { 0, mapValueToRange(normalizeVal(baseHeight, data->stMinH, data->stMaxH), data->acMinH, data->acMaxH) };
	heightData->mid1_pos = { 0, mapValueToRange(normalizeVal(destination->getNpm(), data->stMinH, data->stMaxH), data->acMinH, data->acMaxH) };
	heightData->end_pos = { 0, destination->getNpm() };

	this->name = aircraftNames[type][rand() % aircraftNames[type].size()];
	this->type = type;
}

Ballon::Ballon(glm::fvec2 position, float baseHeight, StaticObj* destination, uint8_t type, acHeightData* data) {
	this->position = position;
	path = { position, destination->getPosition() };

	heightData = new BezierCurveParametersA{};
	heightData->str_pos = { 0, baseHeight };
	heightData->mid0_pos = { 0, mapValueToRange(normalizeVal(baseHeight,data->stMinH, data->stMaxH), data->acMinH, data->acMaxH) };
	heightData->mid1_pos = { 0, mapValueToRange(normalizeVal(destination->getNpm(), data->stMinH, data->stMaxH), data->acMinH, data->acMaxH) };
	heightData->end_pos = { 0, destination->getNpm() };

	this->name = aircraftNames[type][rand() % aircraftNames[type].size()];
	this->type = type;
}

Helicopter::Helicopter(glm::fvec2 position, float baseHeight, StaticObj* destination, uint8_t type, acHeightData* data) {
	this->position = position;
	path = { position, destination->getPosition() };

	heightData = new BezierCurveParametersA{};
	heightData->str_pos = { 0, baseHeight };
	heightData->mid0_pos = { 0, mapValueToRange(normalizeVal(baseHeight,data->stMinH, data->stMaxH), data->acMinH, data->acMaxH) };
	heightData->mid1_pos = { 0, mapValueToRange(normalizeVal(destination->getNpm(),data->stMinH, data->stMaxH), data->acMinH, data->acMaxH) };
	heightData->end_pos = { 0, destination->getNpm() };

	this->name = aircraftNames[type][rand() % aircraftNames[type].size()];
	this->type = type;
}

Jet::Jet(glm::fvec2 position, float baseHeight, StaticObj* destination, uint8_t type, acHeightData* data) {
	this->position = position;
	path = { position, destination->getPosition() };

	distanceToGround = baseHeight;

	heightData = new BezierCurveParametersA{};
	heightData->str_pos = { 0, baseHeight };
	heightData->mid0_pos = { 0, mapValueToRange(normalizeVal(baseHeight, data->stMinH, data->stMaxH), data->acMinH, data->acMaxH) };
	heightData->mid1_pos = { 0, mapValueToRange(normalizeVal(destination->getNpm(),data->stMinH, data->stMaxH), data->acMinH, data->acMaxH) };
	heightData->end_pos = { 0, destination->getNpm() };

	this->name = aircraftNames[type][rand() % aircraftNames[type].size()];
	this->type = type;
}

Plane::Plane(glm::fvec2 position, float baseHeight, StaticObj* destination, uint8_t type, acHeightData* data) {
	this->position = position;
	path = { position, destination->getPosition() };

	heightData = new BezierCurveParametersA{};
	heightData->str_pos = { 0, baseHeight };
	heightData->mid0_pos = { 0, mapValueToRange(normalizeVal(baseHeight, data->stMinH, data->stMaxH), data->acMinH, data->acMaxH) };
	heightData->mid1_pos = { 0, mapValueToRange(normalizeVal(destination->getNpm(), data->stMinH, data->stMaxH), data->acMinH, data->acMaxH) };
	heightData->end_pos = { 0, destination->getNpm() };


	this->name = aircraftNames[type][rand() % aircraftNames[type].size()];
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
	this->start = start;

	UpdatePath();
}

void FlyPath::ChangeDestinatination(glm::vec2 dest) {
	path.at(path.size() - 1).end_pos = dest;
	this->destination = dest;
	UpdatePath();
}


glm::fvec2 FlyPath::getBezierPosition2D(BezierCurveParametersA* param, float dt, bool change) {

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

float FlyPath::getBezierPosition1D(BezierCurveParametersA* param, float dt) {
	float h0 = param->str_pos.y;
	float h1 = param->mid0_pos.y;
	float h2 = param->mid1_pos.y;
	float h3 = param->end_pos.y;

	float Height = (1 - dt) * (1 - dt) * (1 - dt) * h0 + 3 * (1 - dt) * (1 - dt) * dt * h1 + 3 * (1 - dt) * dt * dt * h2 + dt * dt * dt * h3;

	if (t >= 1.0f) {
		assert(currentPathSection < path.size());
		t = 0.0f;
		currentPathSection++;
	}

	return Height;
}

float FlyPath::BezierSingleLength(BezierCurveParametersA* data) {
	glm::fvec2 p0, p1, p2, p3;
	float l0, l1, l2, l3;
	p0 = data->str_pos - data->mid0_pos;
	p1 = data->mid1_pos - data->mid0_pos;
	p3 = data->end_pos - data->mid1_pos;

	l0 = glm::length(p0);
	l1 = glm::length(p1);
	l3 = glm::length(p3);
	if (l0 > 0) p0 /= l0;
	if (l1 > 0) p1 /= l1;
	if (l3 > 0) p3 /= l3;

	p2 = -p1;
	float a = fabs(glm::dot(p0, p1)) + fabs(glm::dot(p2, p3));
	if (a > 1.98f || l0 + l1 + l3 < (4 - a) * 8) return l0 + l1 + l3;

	glm::fvec2 bl[4];
	glm::fvec2 br[4];

	bl[0] = data->str_pos;
	bl[1] = (data->str_pos + data->mid0_pos) * 0.5f;

	glm::fvec2 mid = (data->mid0_pos + data->mid1_pos) * 0.5f;

	bl[2] = (bl[1] + mid) * 0.5f;
	br[3] = data->end_pos;
	br[2] = (data->mid1_pos + data->end_pos) * 0.5f;
	br[1] = (br[2] + mid) * 0.5f;
	br[0] = (br[1] + bl[2]) * 0.5f;
	bl[3] = br[0];

	return BezierSingleLength((BezierCurveParametersA*)&bl) + BezierSingleLength((BezierCurveParametersA*)&br);
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
	//ValidateAnglesNew();
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


//MISC
float normalizeVal(float value, float minValue, float maxValue) {
	if (maxValue == minValue) {
		return 0.0f;
	}
	return (value - minValue) / (maxValue - minValue);
}

float mapValueToRange(float normalizedValue, float minValue, float maxValue) {
	return minValue + normalizedValue * (maxValue - minValue);
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

AircraftRenderData::AircraftRenderData()
{
	model = gltCreateText();
	brand = gltCreateText();
	speed = gltCreateText();
	height = gltCreateText();

	scale = glm::vec2(200.0f, 50.0f);
	posit = glm::vec2(0.0f);
	f = 0.0f;
}

void AircraftRenderData::Render(glm::mat4 _mvp)
{
	gltBeginDraw();

	float w = gltGetTextWidth(model, scale.x);
	float h = gltGetTextHeight(model, scale.y);
	glm::vec2 offset = glm::vec2(0.0f);

	glm::mat4 m = glm::mat4(1.0f);
	m = glm::translate(m, glm::vec3(posit, 2.0f));
	m = glm::translate(m, glm::vec3(w / 2.0f, h / 2.0f, 2.0f));
	m = glm::rotate(m, glm::radians(180.0f), glm::vec3(1, 0, 0));
	m = glm::translate(m, glm::vec3(-w / 2.0f, -h / 2.0f, 0.0f));
	m = glm::scale(m, glm::vec3(scale, 0.0f));

	gltDrawText(model, &(_mvp * m)[0].x);
	offset.y -= h;


	w = gltGetTextWidth(brand, scale.x);
	h = gltGetTextHeight(brand, scale.y);

	m = glm::mat4(1.0f);
	m = glm::translate(m, glm::vec3(posit + offset, 2.0f));
	m = glm::translate(m, glm::vec3(w / 2.0f, h / 2.0f, 2.0f));
	m = glm::rotate(m, glm::radians(180.0f), glm::vec3(1, 0, 0));
	m = glm::translate(m, glm::vec3(-w / 2.0f, -h / 2.0f, 0.0f));
	m = glm::scale(m, glm::vec3(scale, 0.0f));

	gltDrawText(brand, &(_mvp * m)[0].x);
	offset.y -= h;

	w = gltGetTextWidth(speed, scale.x);
	h = gltGetTextHeight(speed, scale.y);

	m = glm::mat4(1.0f);
	m = glm::translate(m, glm::vec3(posit + offset, 2.0f));
	m = glm::translate(m, glm::vec3(w / 2.0f, h / 2.0f, 2.0f));
	m = glm::rotate(m, glm::radians(180.0f), glm::vec3(1, 0, 0));
	m = glm::translate(m, glm::vec3(-w / 2.0f, -h / 2.0f, 0.0f));
	m = glm::scale(m, glm::vec3(scale, 0.0f));

	gltDrawText(speed, &(_mvp * m)[0].x);
	offset.y -= h;

	w = gltGetTextWidth(height, scale.x);
	h = gltGetTextHeight(height, scale.y);

	m = glm::mat4(1.0f);
	m = glm::translate(m, glm::vec3(posit + offset, 2.0f));
	m = glm::translate(m, glm::vec3(w / 2.0f, h / 2.0f, 2.0f));
	m = glm::rotate(m, glm::radians(180.0f), glm::vec3(1, 0, 0));
	m = glm::translate(m, glm::vec3(-w / 2.0f, -h / 2.0f, 0.0f));
	m = glm::scale(m, glm::vec3(scale, 0.0f));

	gltDrawText(height, &(_mvp * m)[0].x);
	offset.y -= h;

	gltEndDraw();
}

void AircraftRenderData::SetModel(const char* v)
{
	std::string s = std::string("Model: ");
	s.append(v);
	gltSetText(model, s.c_str());
}

void AircraftRenderData::SetColor(float r, float g, float b)
{
	gltColor(r, g, b, 0.0f);
}

void AircraftRenderData::SetBrand(const char* v)
{
	std::string s = std::string("Brand: ");
	s.append(v);
	gltSetText(brand, s.c_str());
}

void AircraftRenderData::SetSpeed(float v)
{
	std::string s = std::string("Speed: ");
	s.append(std::to_string(v));
	gltSetText(speed, s.c_str());
}

void AircraftRenderData::SetHeigth(float v)
{
	std::string s = std::string("Height: ");
	s.append(std::to_string(v));
	gltSetText(height, s.c_str());
}

void AircraftRenderData::SetPosition(glm::fvec2 position)
{
	this->posit = position;
}

void AircraftRenderData::SetScale(glm::fvec2 scale)
{
	this->scale = scale;
}
