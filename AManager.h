#include "octree.h"
#include "StaticObjects.h"
#include "Map.h"
#include "Camera.h"
#include <unordered_map>


#ifdef _DEBUG
#define AM_ASSERT(A) assert(A)
#endif

extern glm::mat4 BaseIconScaleMatrix;
extern bool keysPressed[SDL_NUM_SCANCODES];
extern bool wasPressed[SDL_NUM_SCANCODES];

void AddCollisionBuffera(VertexBuffer vao, Buffer<GL_ARRAY_BUFFER> obj);

//extern SDL_Point mousePos;

#define MAP_WIDTH 5000
#define MAP_HEIGHT 5000
#define SCALE 0.5f
#define N_AIRPORTS 1.0f
#define N_TOWERS 1.0f
#define N_AIRCRAFTS 1.0f
#define minNPM 63
#define maxNPM 173

class CollisionDrawer
{
public:
	float* currentArray;
	Buffer<GL_ARRAY_BUFFER>* buffers;
	uint32_t* buffersSize;
	uint32_t bufferCount;
	uint32_t currentArraySize;

	CollisionDrawer() = default;
	CollisionDrawer(uint32_t cnt, uint32_t maxObjects);
	void AddCollisionBuffer(uint32_t type, uint32_t amount, VertexBuffer vao);
	void FetchCollisionBuffer(uint32_t type);
	void SendCollisionBuffer(uint32_t type);

	void UpdateSingleData(uint32_t type, float c, uint32_t idx);
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

		cd = CollisionDrawer(15, 1000);

		int x, y, c;
		Texture2D MapTexture;
		uint8_t* MapTextureData;

		//AIRCRAFTS
		MapTextureData = (uint8_t*)LoadImageData("helicopter.png", 1, &c, &x, &y);
		MapTexture = Texture2D(MapTextureData, x, y, GL_RGBA, GL_RGBA, GL_TEXTURE0);
		addModel(MapTexture, RENDER_MODEL_HELICOPTER);

		MapTextureData = (uint8_t*)LoadImageData("glider.png", 1, &c, &x, &y);
		MapTexture = Texture2D(MapTextureData, x, y, GL_RGBA, GL_RGBA, GL_TEXTURE0);
		addModel(MapTexture, RENDER_MODEL_GLIDER);

		MapTextureData = (uint8_t*)LoadImageData("ballon.png", 1, &c, &x, &y);
		MapTexture = Texture2D(MapTextureData, x, y, GL_RGBA, GL_RGBA, GL_TEXTURE0);
		addModel(MapTexture, RENDER_MODEL_BALLOON);

		MapTextureData = (uint8_t*)LoadImageData("jet.png", 1, &c, &x, &y);
		MapTexture = Texture2D(MapTextureData, x, y, GL_RGBA, GL_RGBA, GL_TEXTURE0);
		addModel(MapTexture, RENDER_MODEL_JET);

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

		//addModel(*map.getMap(), 20);

		//CONFIG
		map = Map{MAP_WIDTH * SCALE, MAP_HEIGHT * SCALE, 1.0, 2.5, 1.0, 7, 21324, m, false};
		
		qtAp._alloc(3);
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
		r->newModel(20, vao, mapProgram, 6, GL_TRIANGLES, map.getMap(), 2);
		r->newObject(20, glm::scale(glm::mat4(1.0f), glm::fvec3{ MAP_WIDTH / 2.0f, MAP_HEIGHT / 2.0f, 0 }));

		generator.gen = std::mt19937(generator.rd());
		//generator.dis = std::uniform_real_distribution<float>(0.0, 1.0);

		qtAc._alloc(5);
		AirCraft* ac;
		//Ballon* ballon = new Ballon{ {0,0}, {0,0} };
		//r.newObject(RENDER_MODEL_BALLON, glm::translate(glm::mat4(1.0f), glm::fvec3{ ballon->position.x, ballon->position.y, 0.05f }) * BaseIconScaleMatrix, &ballon->LongId);

		int aircraftAmount = calculateBestObjectAmount(N_AIRCRAFTS, 2);
		acData.amount = aircraftAmount;
		acData.amin = 0.8f;
		acData.amax = 1.2f;
		for (int i = 0; i < calculateBestObjectAmount(N_AIRCRAFTS, 2); i++) {
			ac = generateRandomAirCraft(i % 5, MAP_WIDTH, MAP_HEIGHT);
			AirCraftMap[ac->getType()].push_back(ac);
			qtAc._push(ac, { ac->position.x, ac->position.y });
		}
		
		//86�400
		/*timeScale = 1.0f / 3600.0f;*/
	}

	void onUpdate() {
		r->RenderSelectedModel(20);
		r->RenderSelectedModel(RENDER_MODEL_AIRPORT);
		r->RenderSelectedModel(RENDER_MODEL_HELICOPTER);
		r->RenderSelectedModel(RENDER_MODEL_TOWER);
		r->RenderSelectedModel(RENDER_MODEL_GLIDER);
		r->RenderSelectedModel(RENDER_MODEL_BALLOON);
		r->RenderSelectedModel(RENDER_MODEL_JET);
		r->RenderSelectedModel(RENDER_MODEL_PLANE);

		if (keyPressedOnce(SDL_SCANCODE_LEFT)) {
			//FOR PERFORMANCE
			glm::fvec2 mousePos = camera->getMousePosition();
			std::cout << mousePos.x << " " << mousePos.y << "\n";
			std::vector<AirCraft*> pcaV;
			if (qtAc._collidePoints(PointQT{ mousePos.x, mousePos.y }, 20, 20, pcaV)) {
				//std::cout << pcaV.at(0)->collide << "|" << pcaV.at(0)->dist << "\n";
				selectedAircraft = pcaV.at(0);
				br->UpdateData((BezierCurveParameters*)(pcaV.at(0)->path.getData()), pcaV.at(0)->path.path.size(), 0);
				pathRenderCount = pcaV.at(0)->path.path.size();
			}
		}
		else if (keyPressedOnce(SDL_SCANCODE_RIGHT)) {
			if (selectedAircraft) {
				glm::fvec2 mousePos = camera->getMousePosition();
				//selectedAircraft->path.AddPoint(mousePos);
				if (selectedAircraft->path.path.size() >= 2) {
					selectedAircraft->path.path.erase(selectedAircraft->path.path.begin() + 0);
				}
				if (selectedAircraft->path.currentPathSection == 1) selectedAircraft->path.currentPathSection--;
				selectedAircraft->path.resetT();
				selectedAircraft->path.AddPoint(mousePos);
				selectedAircraft->path.path.at(0).str_pos = selectedAircraft->position;
				
				//selectedAircraft->path.path.at(0).end_pos = selectedAircraft->path.path.at(1).str_pos;
				//selectedAircraft->path.destination;
				

				br->UpdateData((BezierCurveParameters*)(selectedAircraft->path.getData()), selectedAircraft->path.path.size(), 0);
				pathRenderCount = selectedAircraft->path.path.size();
			}
		}


		//int i = 0;
		//for (auto& ac : AirCraftVec) {
		//	br->UpdateData((BezierCurveParameters*)(ac->path.getData()), ac->path.path.size(), i);
		//	i += ac->path.path.size();
		//}

		handleAirCraftLogic();

	}

private:
	int generateRandomValueRange(int min, int max) {
		std::uniform_int_distribution<> range(min, max);
		return range(generator.gen);
	}

	std::pair<glm::fvec2, glm::fvec2> generateRandomPath(float s) {
		glm::fvec2 start, end;
		int idx0, idx1;

		idx0 = generateRandomValueRange(0, s - 1);
		idx1 = idx0;
		while (idx1 == idx0) {
			idx0 = generateRandomValueRange(0, s - 1);
		}
		return { AirPortsVec.at(idx0)->position, AirPortsVec.at(idx1)->position };
	}

	AirCraft* generateRandomAirCraft(int idx, int mapWidth, int mapHeight) {
		Glider* glider;
		Jet* jet;
		Ballon* ballon;
		Helicopter* helicopter;
		Plane* plane;

		std::pair<glm::fvec2, glm::fvec2> s_e;
		s_e = generateRandomPath(AirPortsVec.size());
		switch (idx)
		{
		case 0:
			ballon = new Ballon{ s_e.first, s_e.second, RENDER_MODEL_BALLOON };
			r->newObject(ballon->getType(), glm::translate(glm::mat4(1.0f), glm::fvec3{ ballon->position.x, ballon->position.y, 0.05f }), &ballon->LongId);
			return (AirCraft*)ballon;
		case 1:
			jet = new Jet{ s_e.first, s_e.second, RENDER_MODEL_JET };
			r->newObject(jet->getType(), glm::translate(glm::mat4(1.0f), glm::fvec3{ jet->position.x, jet->position.y, 0.05f }), &jet->LongId);
			return (AirCraft*)jet;
		case 2:
			helicopter = new Helicopter{ s_e.first, s_e.second, RENDER_MODEL_HELICOPTER };
			r->newObject(helicopter->getType(), glm::translate(glm::mat4(1.0f), glm::fvec3{ helicopter->position.x, helicopter->position.y , 0.05f }), &helicopter->LongId);
			return (AirCraft*)helicopter;
		case 3:
			plane = new Plane{ s_e.first, s_e.second, RENDER_MODEL_PLANE };
			r->newObject(plane->getType(), glm::translate(glm::mat4(1.0f), glm::fvec3{ plane->position.x, plane->position.y, 0.05f }), &plane->LongId);
			return (AirCraft*)plane;
		case 4:
			glider = new Glider{ s_e.first, s_e.second, RENDER_MODEL_GLIDER };
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

		cd.AddCollisionBuffer(idModel, 1000, vao);
		r->newModel(idModel, vao, program, 6, GL_TRIANGLES, texture, 2000);
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

		for (i = 0; i < calculateBestObjectAmount(N_AIRPORTS, 4); i++) {
			position = { generateRandomValueRange(-mapWidth / 2.0f, mapWidth / 2.0f), generateRandomValueRange(-mapHeight / 2.0f, mapHeight / 2.0f) };
			isTerrainAdapted = true;
			for (int y = -50; y < 51; y++) {
				for (int x = -50; x < 51; x++) {
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
				mval = (thr + generateRandomValueRange(0, 20)) * (maxNPM - minNPM) / (255) + minNPM;

				st = new StaticObj{ position, RENDER_MODEL_AIRPORT, mval };
				AirPortsVec.push_back(st);
				qtAp._push(st, { st->position.x, st->position.y });
				r->newObject(st->getType(), glm::translate(glm::mat4(1.0f), glm::fvec3{ st->position.x, st->position.y, 0.05f }), &st->LongId);
			}
		}

		for (i = 0; i < calculateBestObjectAmount(N_TOWERS, 4); i++) {
			position = { generateRandomValueRange(-mapWidth / 2.0f, mapWidth / 2.0f), generateRandomValueRange(-mapHeight / 2.0f, mapHeight / 2.0f) };
			isTerrainAdapted = true;
			for (int y = -50; y < 51; y++) {
				for (int x = -50; x < 51; x++) {
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
				mval = (thr + generateRandomValueRange(0, 20)) * (maxNPM - minNPM) / (255) + minNPM;
				st = new StaticObj{ position, RENDER_MODEL_TOWER,  mval };
				TowersVec.push_back(st);
				qtT._push(st, { st->position.x, st->position.y });
				r->newObject(st->getType(), glm::translate(glm::mat4(1.0f), glm::fvec3{ st->position.x, st->position.y, 0.05f }), &st->LongId);
			}
		}
	}

	void handleAirCraftsMovement(AirCraft*& ac, float t, float z) {
		//float angle = (ac->getType() != RENDER_MODEL_BALLON) ? (ac->CalcAngle() + 1.57079632679) : 0;
		float angle = 0;
		if (ac->getType() != RENDER_MODEL_BALLOON) {
			ac->SetAngle(ac->CalcAngle());
			angle = ac->angle + 1.57079632679;
		}
		ac->position = ac->path.getBezierPosition(ac->path.GetCurrentSection(), t);
		r->BindActiveModel((*(RENDER_LONG_ID*)&ac->LongId).ModelId);
		r->SetObjectMatrix((*(RENDER_LONG_ID*)&ac->LongId).ObjectId, glm::translate(glm::mat4(1.0f), glm::fvec3{ ac->position.x, ac->position.y, 0.05f + z }) * glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0, 0, 1)), true);
	}

	void handleAirCraftCollision(AirCraft*& ac, float w, float h) {
		AirCraft* acr = nullptr;
		// ultra slow shit
		r->BindActiveModel(LONG_GET_MODEL(ac->LongId));

		//ADD THIS TO SETTINGS
		float distMax = 200;
		float distMin = distMax;
		std::vector<AirCraft*> colv;
		if (qtAc._collide(ac, w, h, colv)) {
			for (auto& col : colv) {
				float dist = glm::distance(ac->position, col->position);
				if (dist < distMin) {
					distMin = dist;
				}
			}
			ac->dist = distMin;
			cd.UpdateSingleData(LONG_GET_MODEL(ac->LongId), distMax - distMin, r->MapToObjectIdx(LONG_GET_OBJECT(ac->LongId)));
		}
		else {
			cd.UpdateSingleData(LONG_GET_MODEL(ac->LongId), 0.0f, r->MapToObjectIdx(LONG_GET_OBJECT(ac->LongId)));
		}
	}

	void handleAirCraftLogic() {
		float t = 0.0f;

		int acActualSize = 0;
		std::vector<AirCraft*> AirCraftsToRemove, newAirCrafts;
		float z = 0.0003f;
		for (auto planeType : AirCraftMap) {
			acActualSize += planeType.second.size();
			for (AirCraft* ac : planeType.second) {
				t = 0.0001f;

				handleAirCraftCollision(ac, 200, 200);
				handleAirCraftsMovement(ac, t, z);
				if (glm::distance(ac->position, ac->path.destination) < 3.0f) {
					AirCraftsToRemove.push_back(ac);
				}

				z += 0.01f;
			}
		}
		int acmin, acmax;
		acmin = acData.amin * acData.amount;
		acmax = acData.amax * acData.amount;

		AirCraft* newAc;
		int type;
		for (AirCraft* ac : AirCraftsToRemove) {
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
				type = generateRandomValueRange(0, 4);
				qtAc._push(newAc, { newAc->position.x, newAc->position.y });

				newAc = generateRandomAirCraft(type, MAP_WIDTH, MAP_HEIGHT);
				AirCraftMap[newAc->getType()].push_back(newAc);
				qtAc._push(newAc, { newAc->position.x, newAc->position.y });

			}
		}

		//std::cout << acActualSize << " " << acData.chance << "\n";

		qtAc._clear();
		for (auto planeType : AirCraftMap) {
			for (AirCraft* ac : planeType.second) {
				qtAc._push(ac, { ac->position.x, ac->position.y });
			}
		}
	}

	void rotateAirCraft(AirCraft* airCraft, glm::fvec3 destination) {
		//FOR NOW
		//float angle = 0;

		//airCraft->angle;
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

	std::unordered_map<uint8_t, std::vector<AirCraft*>> AirCraftMap;

	QT<AirCraft> qtAc = { MAP_WIDTH, MAP_HEIGHT };

	std::vector<StaticObj*> AirPortsVec;
	QT<StaticObj> qtAp = { MAP_WIDTH, MAP_HEIGHT };

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
		float amin = 0.8f;
		float amax = 1.2f;
		//For not mapping it all the time
		float chance = 0.5f;
	} acData;
};
