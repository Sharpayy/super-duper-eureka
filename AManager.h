﻿#include "StaticObjects.h"
#include "Map.h"
#include "Camera.h"
#include <unordered_map>
#include "octree.h"

#ifdef _DEBUG
#define AM_ASSERT(A) assert(A)
#endif

extern glm::mat4 BaseIconScaleMatrix;
extern bool keysPressed[SDL_NUM_SCANCODES];
extern bool wasPressed[SDL_NUM_SCANCODES];

void AddCollisionBuffera(VertexBuffer vao, Buffer<GL_ARRAY_BUFFER> obj);

#define MAP_WIDTH 10000
#define MAP_HEIGHT 10000
#define SCALE 0.5f
#define N_AIRPORTS 1.0f
#define N_TOWERS 1.0f
#define N_AIRCRAFTS 1.0f

struct _config {
	float n_airports;
	float n_towers;
	float n_aircrafts;

	//Map
	float map_width;
	float map_height;
	float scale;
	float CPU;
	double persistence;
	double frequency;
	double amplitude;
	int octaves;
	int randomseed;

	//Aircrafts Height
	float aircraftsMinNPM;
	float aircraftsMaxNPM;
	//StaticObjects Height
	float staticObjectsMinNPM;
	float staticObjectsMaxNPM;

	//Collision
	float collsiionDetectionDistance;
	float collisionResponseHeight;
	float airportsCollisionDistance;
	float towersCollisionDistance;

};

class CollisionDrawer
{
public:
	float* mappedArray;
	Buffer<GL_ARRAY_BUFFER> buffer;

	CollisionDrawer() = default;
	CollisionDrawer(uint32_t objs);
	void AddCollisionBuffer(uint32_t amount, VertexBuffer vao);
	void FetchCollisionBuffer();
	void SendCollisionBuffer();

	void UpdateSingleData(uint32_t type, float c, uint32_t idx);
private:
	uint32_t offset;
	uint32_t size;
};
class AManager {
public:
	AManager() = default;
	AManager(RenderGL* r, Buffer<GL_ARRAY_BUFFER> vbo, Program program, Program mapProgram, BezierRenderer* br, Buffer<GL_ELEMENT_ARRAY_BUFFER> ebo, Camera* camera, Program m) {
		this->r = r;
		this->vbo = vbo;
		this->ebo = ebo;
		this->program = program;
		this->br = br;
		this->camera = camera;

		pathRenderCount = 0;
		int aircraftAmount = calculateBestObjectAmount(N_AIRCRAFTS, 2);
		cd = CollisionDrawer(500 * 5);

		int x, y, c;
		Texture2D MapTexture;
		uint8_t* MapTextureData;

		program.use();
		ls = glGetUniformLocation(program.id, "uIconScale");

		//AIRCRAFTS
		MapTextureData = (uint8_t*)LoadImageData("ballon.png", 1, &c, &x, &y);
		MapTexture = Texture2D(MapTextureData, x, y, GL_RGBA, GL_RGBA, GL_TEXTURE0);
		addModel(MapTexture, RENDER_MODEL_BALLOON);

		MapTextureData = (uint8_t*)LoadImageData("jet.png", 1, &c, &x, &y);
		MapTexture = Texture2D(MapTextureData, x, y, GL_RGBA, GL_RGBA, GL_TEXTURE0);
		addModel(MapTexture, RENDER_MODEL_JET);

		MapTextureData = (uint8_t*)LoadImageData("helicopter.png", 1, &c, &x, &y);
		MapTexture = Texture2D(MapTextureData, x, y, GL_RGBA, GL_RGBA, GL_TEXTURE0);
		addModel(MapTexture, RENDER_MODEL_HELICOPTER);

		MapTextureData = (uint8_t*)LoadImageData("glider.png", 1, &c, &x, &y);
		MapTexture = Texture2D(MapTextureData, x, y, GL_RGBA, GL_RGBA, GL_TEXTURE0);
		addModel(MapTexture, RENDER_MODEL_GLIDER);

		MapTextureData = (uint8_t*)LoadImageData("plane.png", 1, &c, &x, &y);
		MapTexture = Texture2D(MapTextureData, x, y, GL_RGBA, GL_RGBA, GL_TEXTURE0);
		addModel(MapTexture, RENDER_MODEL_PLANE);

		//STATIC OBJECTS
		MapTextureData = (uint8_t*)LoadImageData("airport.png", 1, &c, &x, &y);
		MapTexture = Texture2D(MapTextureData, x, y, GL_RGBA, GL_RGBA, GL_TEXTURE0);
		addModel(MapTexture, RENDER_MODEL_AIRPORT);

		MapTextureData = (uint8_t*)LoadImageData("tower.png", 1, &c, &x, &y);
		MapTexture = Texture2D(MapTextureData, x, y, GL_RGBA, GL_RGBA, GL_TEXTURE0);
		addModel(MapTexture, RENDER_MODEL_TOWER);

		//ETC
		MapTextureData = (uint8_t*)LoadImageData("Explosion.png", 1, &c, &x, &y);
		MapTexture = Texture2D(MapTextureData, x, y, GL_RGBA, GL_RGBA, GL_TEXTURE0);
		addModel(MapTexture, RENDER_MODEL_EXPLOSION);

		//addModel(*map.getMap(), 20);

		//CONFIG
		map = Map{ MAP_WIDTH * SCALE, MAP_HEIGHT * SCALE, 1.0, 2.5, 1.0, 7, 21324, m, true };

		qtAp._alloc(2);
		generateStaticObjects(MAP_WIDTH, MAP_HEIGHT);
		map.ReleaseTextureData();

		VertexBuffer vao = VertexBuffer();
		vao.bind();
		vbo.bind();
		ebo.bind();

		vao.addAttrib(GL_FLOAT, 0, 2, sizeof(float) * 4, 0);
		vao.addAttrib(GL_FLOAT, 1, 2, sizeof(float) * 4, 8);
		vao.enableAttrib(0);
		vao.enableAttrib(1);
		r->newModel(RENDER_MODEL_MAP, vao, mapProgram, 6, GL_TRIANGLES, map.getMap(), 2);
		r->newObject(RENDER_MODEL_MAP, glm::scale(glm::mat4(1.0f), glm::fvec3{ MAP_WIDTH / 2.0f, MAP_HEIGHT / 2.0f, 0 }));

		generator.gen = std::mt19937(generator.rd());
		//generator.dis = std::uniform_real_distribution<float>(0.0, 1.0);

		qtAc._alloc(5);
		AirCraft* ac;

		acData.amount = aircraftAmount;
		acData.amin = 0.8f;
		acData.amax = 1.2f;

		for (int i = 0; i < aircraftAmount; i++) {
			ac = generateRandomAirCraft(i % 5, MAP_WIDTH, MAP_HEIGHT);
			AirCraftMap[ac->getType()].push_back(ac);
			wMap[ac->LongId] = ac;
			//float sag = glm::distance(ac->path.path.at(ac->path.currentPathSection).str_pos, ac->path.path.at(ac->path.currentPathSection).end_pos);
			//float g = ac->path.BezierSingleLength(ac->path.getData());
			//float h = 1.0f;
			//qtAc._push(ac, { ac->position.x, ac->position.y });
		}


		//TEXT
		ard.SetColor(0.7f, 0.2f, 0.1f);

		//86�400
		/*timeScale = 1.0f / 3600.0f;*/
	}

	void onUpdate(float dt) {
		if (selectedAircraft) std::cout << selectedAircraft->collide << " -1 \n";
		r->RenderSelectedModel(RENDER_MODEL_MAP);
		r->RenderSelectedModel(RENDER_MODEL_EXPLOSION);
		glUniform1f(ls, 10.0f);
		r->RenderSelectedModel(RENDER_MODEL_AIRPORT);
		r->RenderSelectedModel(RENDER_MODEL_HELICOPTER);
		r->RenderSelectedModel(RENDER_MODEL_TOWER);
		r->RenderSelectedModel(RENDER_MODEL_GLIDER);
		r->RenderSelectedModel(RENDER_MODEL_BALLOON);
		r->RenderSelectedModel(RENDER_MODEL_JET);
		r->RenderSelectedModel(RENDER_MODEL_PLANE);
		if (r->getModel(RENDER_MODEL_EXPLOSION)->activeObjects != 0) glUniform1f(ls, 50.0f);

		//TEXT
		if (selectedAircraft) {
			ard.SetPosition({ selectedAircraft->position.x - 40, selectedAircraft->position.y - 40 });
			ard.SetScale({ 0.7f,0.7f });
			ard.SetHeigth(selectedAircraft->distanceToGround);
			ard.SetSpeed(selectedAircraft->speed);
			ard.Render(r->MVP.matProjCamera);
		}

		if (keyPressedOnce(SDL_SCANCODE_LEFT)) {
			//FOR PERFORMANCE
			glm::fvec2 mousePos = camera->getMousePosition();
			std::cout << mousePos.x << " " << mousePos.y << "\n";
			std::vector<AirCraft*> pcaV;
			if (qtAc._collidePoints(PointQT{ mousePos.x, mousePos.y }, 50, 50, pcaV)) {
				//std::cout << pcaV.at(0)->collide << "|" << pcaV.at(0)->dist << "\n";
				selectedAircraft = pcaV.at(0);
				ard.SetModel(selectedAircraft->GetName().c_str());
				ard.SetBrand("Lot");

				br->UpdateData((BezierCurveParameters*)(pcaV.at(0)->path.getData()), pcaV.at(0)->path.path.size(), 0);
				pathRenderCount = pcaV.at(0)->path.path.size();
			}
			else {
				br->UpdateData(nullptr, 0, 0);
				selectedAircraft = nullptr;
			}
		}
		else if (keyPressedOnce(SDL_SCANCODE_RIGHT)) {
			if (selectedAircraft) {
				glm::fvec2 mousePos = camera->getMousePosition();

				std::vector<StaticObj*> objects;
				StaticObj* obj = nullptr;
				qtAp._collidePoints(PointQT{ mousePos.x, mousePos.y }, 10.0f, 10.0f, objects);
				if (objects.size()) {
					obj = objects.at(0);
					selectedAircraft->path.path.at(selectedAircraft->path.currentPathSection).str_pos = selectedAircraft->position;
					selectedAircraft->path.ChangeDestinatination(obj->getPosition());
					selectedAircraft->path.resetT();

					selectedAircraft->heightData->str_pos = { 0, selectedAircraft->distanceToGround };
					selectedAircraft->heightData->end_pos = { 0, obj->getNpm() };
					br->UpdateData((BezierCurveParameters*)(selectedAircraft->path.getData()), selectedAircraft->path.path.size(), 0);
					pathRenderCount = selectedAircraft->path.path.size();
				}
				else {
					qtT._collidePoints(PointQT{ mousePos.x, mousePos.y }, 10.0f, 10.0f, objects);
					if (objects.size()) {
						obj = objects.at(0);
						selectedAircraft->path.path.at(selectedAircraft->path.currentPathSection).str_pos = selectedAircraft->position;
						selectedAircraft->path.ChangeDestinatination(obj->getPosition());
						selectedAircraft->path.resetT();
						selectedAircraft->heightData->str_pos = { 0, selectedAircraft->distanceToGround };
						selectedAircraft->heightData->end_pos = { 0, obj->getNpm() };
						br->UpdateData((BezierCurveParameters*)(selectedAircraft->path.getData()), selectedAircraft->path.path.size(), 0);
						pathRenderCount = selectedAircraft->path.path.size();
					}
				}

				if (!obj) {
					if (selectedAircraft->path.path.size() >= 2) {
						selectedAircraft->path.path.erase(selectedAircraft->path.path.begin() + 0);
					}
					if (selectedAircraft->path.currentPathSection == 1) selectedAircraft->path.currentPathSection--;
					selectedAircraft->path.resetT();
					selectedAircraft->path.AddPoint(mousePos);
					selectedAircraft->path.path.at(0).str_pos = selectedAircraft->position;
					br->UpdateData((BezierCurveParameters*)(selectedAircraft->path.getData()), selectedAircraft->path.path.size(), 0);
					pathRenderCount = selectedAircraft->path.path.size();
				}
			}
		}
		handleAirCraftLogic(dt);
	}

private:
	int generateRandomValueRange(int min, int max) {
		std::uniform_int_distribution<> range(min, max);
		return range(generator.gen);
	}

	std::pair<StaticObj*, StaticObj*> generateRandomPath(float s) {
		glm::fvec2 start, end;
		int idx0, idx1;

		idx0 = generateRandomValueRange(0, s - 1);
		idx1 = idx0;
		while (idx1 == idx0) {
			idx0 = generateRandomValueRange(0, s - 1);
		}
		return { AirPortsVec.at(idx0), AirPortsVec.at(idx1) };
	}

	AirCraft* generateRandomAirCraft(int idx, int mapWidth, int mapHeight) {
		Glider* glider;
		Jet* jet;
		Ballon* ballon;
		Helicopter* helicopter;
		Plane* plane;

		std::pair<StaticObj*, StaticObj*> s_e;
		s_e = generateRandomPath(AirPortsVec.size());
		switch (idx)
		{
		case 0:
			ballon = new Ballon{ s_e.first->getPosition(),  (float)s_e.first->getNpm(), s_e.second, RENDER_MODEL_BALLOON };
			r->newObject(ballon->getType(), glm::translate(glm::mat4(1.0f), glm::fvec3{ ballon->position.x, ballon->position.y, 0.05f }), &ballon->LongId);
			return (AirCraft*)ballon;
		case 1:
			jet = new Jet{ s_e.first->getPosition(), (float)s_e.first->getNpm(), s_e.second, RENDER_MODEL_JET };
			r->newObject(jet->getType(), glm::translate(glm::mat4(1.0f), glm::fvec3{ jet->position.x, jet->position.y, 0.05f }), &jet->LongId);
			return (AirCraft*)jet;
		case 2:
			helicopter = new Helicopter{ s_e.first->getPosition(),  (float)s_e.first->getNpm(),  s_e.second, RENDER_MODEL_HELICOPTER };
			r->newObject(helicopter->getType(), glm::translate(glm::mat4(1.0f), glm::fvec3{ helicopter->position.x, helicopter->position.y , 0.05f }), &helicopter->LongId);
			return (AirCraft*)helicopter;
		case 3:
			plane = new Plane{ s_e.first->getPosition(),  (float)s_e.first->getNpm(), s_e.second, RENDER_MODEL_PLANE };
			r->newObject(plane->getType(), glm::translate(glm::mat4(1.0f), glm::fvec3{ plane->position.x, plane->position.y, 0.05f }), &plane->LongId);
			return (AirCraft*)plane;
		case 4:
			glider = new Glider{ s_e.first->getPosition(),  (float)s_e.first->getNpm(), s_e.second, RENDER_MODEL_GLIDER };
			r->newObject(glider->getType(), glm::translate(glm::mat4(1.0f), glm::fvec3{ glider->position.x, glider->position.y, 0.05f }), &glider->LongId);
			return (AirCraft*)glider;

		default:
			break;
		}
	}

	void addModel(Texture2D texture, uint8_t idModel) {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		texture.genMipmap();

		VertexBuffer vao = VertexBuffer();
		vao.bind();
		vbo.bind();
		ebo.bind();

		vao.addAttrib(GL_FLOAT, 0, 2, sizeof(float) * 4, 0);
		vao.addAttrib(GL_FLOAT, 1, 2, sizeof(float) * 4, 8);
		vao.enableAttrib(0);
		vao.enableAttrib(1);

		cd.AddCollisionBuffer(500, vao);
		r->newModel(idModel, vao, program, 6, GL_TRIANGLES, texture, 1000);
	}

	void generateStaticObjects(int mapWidth, int mapHeight) {
		int i;
		StaticObj* st;
		glm::fvec2 position;

		//CHANGE IT LATER AND ADD SETTINGS
		uint8_t biomeType;
		bool isTerrainAdapted;
		float thr;
		uint8_t mval;
		for (i = 0; i < calculateBestObjectAmount(N_AIRPORTS, 5); i++) {
			position = { generateRandomValueRange(-mapWidth / 2.0f, mapWidth / 2.0f), generateRandomValueRange(-mapHeight / 2.0f, mapHeight / 2.0f) };
			isTerrainAdapted = true;
			for (int y = -20; y < 21; y++) {
				for (int x = -20; x < 21; x++) {
					biomeType = map.getBiomeTypeAdv(glm::clamp(position.x + x, -mapWidth / 2.0f, mapWidth / 2.0f - 1), glm::clamp(position.y + y, -mapHeight / 2.0f, mapHeight / 2.0f - 1), SCALE);
					if (biomeType == WATER || biomeType == HILL || biomeType == MOUNTAIN || biomeType == BEACH || biomeType == SWAMP || biomeType == SNOW) {
						isTerrainAdapted = false;
						break;
					}
					if (x == 0 && y == 0) thr = map.getBiomeThr(glm::clamp(position.x, -mapWidth / 2.0f, mapWidth / 2.0f - 1), glm::clamp(position.y, -mapHeight / 2.0f, mapHeight / 2.0f - 1), SCALE);
				}
			}
			if (!isTerrainAdapted) continue;
			if (!qtAp._collidePoint(PointQT{ position.x, position.y }, 300, 300)) {
				mval = (thr + generateRandomValueRange(0, 20)) * (staticMaxNPM - staticMinNPM) / (255) + staticMinNPM;

				st = new StaticObj{ position, RENDER_MODEL_AIRPORT, mval };
				AirPortsVec.push_back(st);
				position = st->getPosition();
				qtAp._push(st, { position.x, position.y });
				r->newObject(st->getType(), glm::translate(glm::mat4(1.0f), glm::fvec3{ position.x, position.y, 0.05f }), &st->LongId);
			}
		}

		for (i = 0; i < calculateBestObjectAmount(N_TOWERS, 4); i++) {
			position = { generateRandomValueRange(-mapWidth / 2.0f, mapWidth / 2.0f), generateRandomValueRange(-mapHeight / 2.0f, mapHeight / 2.0f) };
			isTerrainAdapted = true;
			for (int y = -25; y < 26; y++) {
				for (int x = -25; x < 26; x++) {
					biomeType = map.getBiomeTypeAdv(glm::clamp(position.x + x, -mapWidth / 2.0f, mapWidth / 2.0f - 1), glm::clamp(position.y + y, -mapHeight / 2.0f, mapHeight / 2.0f - 1), SCALE);
					if (biomeType == WATER || biomeType == HILL || biomeType == MOUNTAIN || biomeType == BEACH || biomeType == SWAMP || biomeType == SNOW) {
						isTerrainAdapted = false;
						break;
					}
					if (x == 0 && y == 0) thr = map.getBiomeThr(glm::clamp(position.x, -mapWidth / 2.0f, mapWidth / 2.0f - 1), glm::clamp(position.y, -mapHeight / 2.0f, mapHeight / 2.0f - 1), SCALE);
				}
			}
			if (!isTerrainAdapted) continue;
			//ADD THIS TO CONFIG
			if (!qtT._collidePoint(PointQT{ position.x, position.y }, 100, 100) && !qtAp._collidePoint(PointQT{ position.x, position.y }, 100, 100)) {
				mval = (thr + generateRandomValueRange(0, 20)) * (staticMaxNPM - staticMinNPM) / (255) + staticMinNPM;
				st = new StaticObj{ position, RENDER_MODEL_TOWER,  mval };
				TowersVec.push_back(st);
				position = st->getPosition();
				qtT._push(st, { position.x, position.y });
				r->newObject(st->getType(), glm::translate(glm::mat4(1.0f), glm::fvec3{ position.x, position.y, 0.05f }), &st->LongId);
			}
		}
	}

	void handleAirCraftsMovement(AirCraft*& ac, float dt) {
		float maxDist = 0, angle;

		float pathDistance = ac->path.BezierSingleLength(ac->path.GetCurrentSection());
		float t = ac->speed / pathDistance * dt / 1000.0f * 40000.0f;
		ac->position = ac->path.getBezierPosition2D(ac->path.GetCurrentSection(), t);
		if (ac->path.path.size() == 1) maxDist = glm::distance(ac->path.start, ac->path.destination);
		else {
			int i;
			for (i = 0; i < ac->path.path.size() - 1; i++) {
				maxDist += glm::distance(ac->path.path.at(i).str_pos, ac->path.path.at(i + 1).str_pos);
			}
			maxDist += glm::distance(ac->path.path.at(i).str_pos, ac->path.path.at(i).end_pos);
		}

		t = glm::clamp(1.0f - (glm::distance(ac->position, ac->path.destination) / maxDist), 0.0f, 1.0f);
		angle = getAirCraftAngle(ac);
		ac->distanceToGround = ac->path.getBezierPosition1D(ac->heightData, t);
		r->BindActiveModel((*(RENDER_LONG_ID*)&ac->LongId).ModelId);
		r->SetObjectMatrix((*(RENDER_LONG_ID*)&ac->LongId).ObjectId, glm::translate(glm::mat4(1.0f), glm::fvec3{ ac->position.x, ac->position.y, 0.05f }) * glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0, 0, 1)), true);
	}

	void handleAirCraftCollision(AirCraft*& ac, float w, float h, std::vector<AirCraft*>& AircraftsToRemove) {
		AirCraft* acr = nullptr;
		// ultra slow shit
		//ADD THIS TO SETTINGS
		float dist, hd = 1000;
		float distMax = 200;
		float distMin = distMax;
		std::vector<AirCraft*> colv;
		bool same = false;
		bool collide;
		if (qtAc._getSize()) collide = qtAc._collidePoints(PointQT{ ac->position.x, ac->position.y }, w, h, colv);
		else {
			wMap.erase(ac->LongId);
			collide = qtAc._collidePoints(PointQT{ ac->position.x, ac->position.y }, w, h, colv);
			qtAc._push(ac, { ac->position.x, ac->position.y });
		}
		if (collide) {
			if (!elementExist(ac->LongId)) {
				for (auto& col : colv) {
					if (col != ac) {
						dist = glm::distance(ac->position, col->position);
						if (dist < distMin) {
							distMin = dist;
						}
						dist = glm::distance(ac->distanceToGround, col->distanceToGround);
						if (dist < hd) hd = dist;
					}
					else same = true;
				}
				ac->dist = distMin;
				if ((colv.size() >= 2)) {
					if (ac->dist < 1.5f && hd < 50.0f) {
						r->newObject(RENDER_MODEL_EXPLOSION, glm::translate(glm::mat4(1.0f), glm::fvec3{ ac->position.x, ac->position.y, 0.05f }));
						AircraftsToRemove.push_back(ac);
						std::cout << "BOOM KURWA\n";
					}
					cd.UpdateSingleData(LONG_GET_MODEL(ac->LongId) * 500.0f, glm::clamp(distMax - distMin, 0.0f, distMax), r->MapToObjectIdx(LONG_GET_OBJECT(ac->LongId)));
				}
				else if (same) cd.UpdateSingleData(LONG_GET_MODEL(ac->LongId) * 500.0f, 0.0f, r->MapToObjectIdx(LONG_GET_OBJECT(ac->LongId)));

			}
		}
		else {
			if (elementExist(ac->LongId)) {
				wMap.erase(ac->LongId);
				qtAc._push(ac, { ac->position.x, ac->position.y });
			}
		}
	}

	void handleAirCraftLogic(float t) {
		cd.FetchCollisionBuffer();
		int acActualSize = 0;
		std::vector<AirCraft*> AirCraftsToRemove, newAirCrafts;
		for (auto planeType : AirCraftMap) {
			acActualSize += planeType.second.size();
			for (AirCraft* ac : planeType.second) {
				r->BindActiveModel(LONG_GET_MODEL(ac->LongId));

				handleAirCraftCollision(ac, 200, 200, AirCraftsToRemove);
				if (!elementExist(ac->LongId)) {
					handleAirCraftsMovement(ac, t);
				}
				if (glm::distance(ac->position, ac->path.destination) < 5.0f) {
					AirCraftsToRemove.push_back(ac);
				}
			}
		}
		int acmin, acmax;
		acmin = acData.amin * acData.amount;
		acmax = acData.amax * acData.amount;

		AirCraft* newAc;
		int type;
		for (AirCraft* ac : AirCraftsToRemove) {
			if (ac == selectedAircraft) {
				br->UpdateData(nullptr, 0, 0);
				selectedAircraft = nullptr;
			}
			r->DisableObjectL(ac->LongId);
			r->deleteObject((*(RENDER_LONG_ID*)&ac->LongId).ModelId, (*(RENDER_LONG_ID*)&ac->LongId).ObjectId);
			auto& vec = AirCraftMap[ac->getType()];
			vec.erase(std::remove(vec.begin(), vec.end(), ac), vec.end());
			acData.chance = 1 - (acActualSize - 1 - acmin) / (float)(acmax - acmin);
			std::uniform_real_distribution<> dis(0.0, 1.0);
			if (dis(generator.gen) < (acData.chance + 0.1f)) {
				type = generateRandomValueRange(0, 4);
				newAc = generateRandomAirCraft(type, MAP_WIDTH, MAP_HEIGHT);
				AirCraftMap[newAc->getType()].push_back(newAc);
				wMap[newAc->LongId] = newAc;

				type = generateRandomValueRange(0, 4);
				newAc = generateRandomAirCraft(type, MAP_WIDTH, MAP_HEIGHT);
				AirCraftMap[newAc->getType()].push_back(newAc);
				wMap[newAc->LongId] = newAc;

			}
		}

		qtAc._clear();
		for (auto planeType : AirCraftMap) {
			for (AirCraft* ac : planeType.second) {
				if (!elementExist(ac->LongId)) {
					qtAc._push(ac, { ac->position.x, ac->position.y });
				}
			}
		}
		cd.SendCollisionBuffer();
	}

	float getAirCraftAngle(AirCraft* ac) {
		float angle = 0;
		if (ac->getType() != RENDER_MODEL_BALLOON) {
			ac->SetAngle(ac->CalcAngle());
			angle = ac->angle + 1.57079632679;
		}
		return angle;
	}

	bool keyPressedOnce(SDL_Scancode key) {
		if (keysPressed[key]) {
			if (wasPressed[key] == true) return false;
			wasPressed[key] = true;
			return true;
		}
		else {
			wasPressed[key] = false;
		}
		return false;
	}

private:
	int calculateBestObjectAmount(float factor, float def) {
		float exmpl = 1000 * 1000;
		float amount = MAP_WIDTH * MAP_HEIGHT / exmpl;
		return amount * factor * def;
	}

	bool elementExist(uint64_t id) {
		return !(wMap.find(id) == wMap.end());
	}

	std::unordered_map<uint8_t, std::vector<AirCraft*>> AirCraftMap;
	std::unordered_map<uint64_t, AirCraft*> wMap;

	QT<AirCraft> qtAc = { MAP_WIDTH, MAP_HEIGHT };

	std::vector<StaticObj*> AirPortsVec;
	QT<StaticObj> qtAp = { MAP_WIDTH, MAP_HEIGHT };
	uint32_t ls;

	std::vector<StaticObj*> TowersVec;
	QT<StaticObj> qtT = { MAP_WIDTH, MAP_HEIGHT };

	RenderGL* r;
	Buffer<GL_ARRAY_BUFFER> vbo;
	Buffer<GL_ELEMENT_ARRAY_BUFFER> ebo;
	Program program;

	Map map;
	Camera* camera;

	BezierRenderer* br;
	CollisionDrawer cd;
	uint32_t pathRenderCount;

	struct _generator {
		std::random_device rd;
		std::mt19937 gen;
		//std::uniform_real_distribution<float> dis;
	} generator;
	//FOR NOW

	AirCraft* selectedAircraft = nullptr;

	struct aircraftAmountData {
		int amount;
		float amin = 1.0f;//0.8f;
		float amax = 1.0f;//1.2f;
		//For not mapping it all the time
		float chance = 0.5f;
	} acData;

	AircraftRenderData ard = AircraftRenderData();
};