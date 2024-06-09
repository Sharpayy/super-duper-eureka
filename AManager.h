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

#define MAP_WIDTH 10000
#define MAP_HEIGHT 10000
#define SCALE 50
#define MAP_OFFSETX 0
#define MAP_OFFSETY 0
#define N_AIRPORTS 1000
#define N_TOWERS 200
#define N_AIRCRAFTS 1000

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
	AManager(RenderGL* r, Buffer<GL_ARRAY_BUFFER> vbo, Program program, Program mapProgram, BezierRenderer* br, Buffer<GL_ELEMENT_ARRAY_BUFFER> ebo, Camera* camera) {
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

		map = Map{ MAP_WIDTH / SCALE, MAP_HEIGHT / SCALE, 1.0, 2.5, 1.0, 7, 21324 };
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

		//AIRCRAFTS
		MapTextureData = (uint8_t*)LoadImageData("helicopter.png", 1, &c, &x, &y);
		MapTexture = Texture2D(MapTextureData, x, y, GL_RGBA, GL_RGBA, GL_TEXTURE0);
		addModel(MapTexture, RENDER_MODEL_HELICOPTER);

		MapTextureData = (uint8_t*)LoadImageData("glider.png", 1, &c, &x, &y);
		MapTexture = Texture2D(MapTextureData, x, y, GL_RGBA, GL_RGBA, GL_TEXTURE0);
		addModel(MapTexture, RENDER_MODEL_GLIDER);

		MapTextureData = (uint8_t*)LoadImageData("ballon.png", 1, &c, &x, &y);
		MapTexture = Texture2D(MapTextureData, x, y, GL_RGBA, GL_RGBA, GL_TEXTURE0);
		addModel(MapTexture, RENDER_MODEL_BALLON);

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

		qtAp._alloc(2);
		generateStaticObjects(MAP_WIDTH, MAP_HEIGHT);

		qtAc._alloc(8);
		AirCraft* ac;
		//Ballon* ballon = new Ballon{ {0,0}, {0,0} };
		//r.newObject(RENDER_MODEL_BALLON, glm::translate(glm::mat4(1.0f), glm::fvec3{ ballon->position.x, ballon->position.y, 0.05f }) * BaseIconScaleMatrix, &ballon->LongId);
		for (int i = 0; i < N_AIRCRAFTS; i++) {
			ac = generateRandomAirCraft(i % 4 + 1, MAP_WIDTH, MAP_HEIGHT);
			ac->path.AddPoint(vec2(120.0f, -140.0f));
			//ac->path.AddPoint(vec2(0,0));
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
		r->RenderSelectedModel(RENDER_MODEL_BALLON);
		r->RenderSelectedModel(RENDER_MODEL_JET);
		r->RenderSelectedModel(RENDER_MODEL_PLANE);

		if (keyPressedOnce(SDL_SCANCODE_LEFT)) {
			//FOR PERFORMANCE
			glm::fvec2 mousePos = camera->getMousePosition();
			//std::cout << mousePos.x << "|" << mousePos.y << "\n";
			std::vector<AirCraft*> pcaV;
			if (qtAc._collidePoints(PointQT{ mousePos.x, mousePos.y }, 20, 20, pcaV)) {
				//std::cout << pcaV.at(0)->collide << "|" << pcaV.at(0)->dist << "\n";
				br->UpdateData((BezierCurveParameters*)(pcaV.at(0)->path.getData()), pcaV.at(0)->path.path.size(), 0);
				pathRenderCount = pcaV.at(0)->path.path.size();
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
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_int_distribution<> range(min, max);
		return range(gen);
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
			ballon = new Ballon{ s_e.first, s_e.second, RENDER_MODEL_BALLON };
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
		r->newModel(idModel, vao, program, 6, GL_TRIANGLES, texture, 1000);
	}

	void generateStaticObjects(int mapWidth, int mapHeight) {
		int i;
		StaticObj* st;
		glm::fvec2 position;

		//CHANGE IT LATER AND ADD SETTINGS

		float dx, dy;
		for (i = 0; i < N_AIRPORTS; i++) {
			//position = { mapWidth / 2.0f, mapHeight / 2.0f };
			position = { generateRandomValueRange(-mapWidth / 2.0f, mapWidth / 2.0f), generateRandomValueRange(-mapHeight / 2.0f, mapHeight / 2.0f) };
			st = new StaticObj{ position, RENDER_MODEL_AIRPORT };

			uint8_t* a = map.GetTile(position.x, position.y);
			uint8_t v[3] = { (uint8_t)0.0f, (uint8_t)105.0f, (uint8_t)148.0f };

			if (memcmp(a, v, 3) != 0)
				continue;

			if (!qtAp._collidePoint(PointQT{ position.x, position.y }, 10, 10)) {
				AirPortsVec.push_back(st);
				qtAp._push(st, { st->position.x, st->position.y });
				r->newObject(st->getType(), glm::translate(glm::mat4(1.0f), glm::fvec3{ st->position.x, st->position.y, 0.05f }), &st->LongId);
			}
		}

		for (i = 0; i < N_TOWERS; i++) {
			position = { generateRandomValueRange(-mapWidth / 2.0f, mapWidth / 2.0f), generateRandomValueRange(-mapHeight / 2.0f, mapHeight / 2.0f) };
			st = new StaticObj{ position, RENDER_MODEL_TOWER };

			//ADD THIS TO CONFIG
			if (!qtT._collidePoint(PointQT{ position.x, position.y }, 25, 25)) {
				TowersVec.push_back(st);
				qtT._push(st, { st->position.x, st->position.y });
				r->newObject(st->getType(), glm::translate(glm::mat4(1.0f), glm::fvec3{ st->position.x, st->position.y, 0.05f }), &st->LongId);
			}
		}
	}

	void handleAirCraftsMovement(AirCraft*& ac, float t, float z) {
		//float angle = (ac->getType() != RENDER_MODEL_BALLON) ? (ac->CalcAngle() + 1.57079632679) : 0;
		float angle = 0;
		if (ac->getType() != RENDER_MODEL_BALLON) {
			ac->SetAngle(ac->CalcAngle());
			angle = ac->angle + 1.57079632679;
		}
		ac->position = ac->path.getBezierPosition(ac->path.GetCurrentSection(), t);
		r->BindActiveModel((*(RENDER_LONG_ID*)&ac->LongId).ModelId);
		r->SetObjectMatrix((*(RENDER_LONG_ID*)&ac->LongId).ObjectId, glm::translate(glm::mat4(1.0f), glm::fvec3{ac->position.x, ac->position.y, 0.05f + z}) * glm::rotate(glm::mat4(1.0f), angle , glm::vec3(0, 0, 1)), true);
	}

	void handleAirCraftCollision(AirCraft*& ac, float w, float h) {
		AirCraft* acr = nullptr;
		// ultra slow shit
		r->BindActiveModel(LONG_GET_MODEL(ac->LongId));

		//ADD THIS TO SETTINGS
		float distMax = 60;
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

		std::vector<AirCraft*> AirCraftsToRemove;
		float z = 0.0003f;
		for (auto planeType : AirCraftMap) {
			for (AirCraft* ac : planeType.second) {
				t = 0.002f;

				handleAirCraftCollision(ac, 60, 60);
				handleAirCraftsMovement(ac, t, z);
				if (glm::distance(ac->position, ac->path.destination) < 3.0f) {
					AirCraftsToRemove.push_back(ac);
				}

				z += 0.01f;
			}
		}

		for (AirCraft* ac : AirCraftsToRemove) {
			r->DisableObjectL(ac->LongId);
			r->deleteObject((*(RENDER_LONG_ID*)&ac->LongId).ModelId, (*(RENDER_LONG_ID*)&ac->LongId).ObjectId);
			auto& vec = AirCraftMap[ac->getType()];
			vec.erase(std::remove(vec.begin(), vec.end(), ac), vec.end());
		}

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
	std::unordered_map<uint8_t,std::vector<AirCraft*>> AirCraftMap;

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

	//FOR NOW
};