#include "Bezier.h"
#include "octree.h"
#include "StaticObjects.h"

#ifdef _DEBUG
#define AM_ASSERT(A) assert(A)
#endif

extern glm::mat4 BaseIconScaleMatrix;

SDL_Point mousePos;

class AManager {
public:
	std::vector<AirCraft*> AirCraftVec;
	AManager() = default;
	AManager(RenderGL& r, VertexBuffer& vertexBuff, Program program, BezierRenderer& br) {
		this->r = r;
		this->vertexBuff = vertexBuff;
		this->program = program;
		this->br = br;

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


		qtAp._alloc(2);
		generateStaticObjects(800, 800);

		qtAc._alloc(8);
		AirCraft* ac;
		//Ballon* ballon = new Ballon{ {0,0}, {0,0} };
		//r.newObject(RENDER_MODEL_BALLON, glm::translate(glm::mat4(1.0f), glm::fvec3{ ballon->position.x, ballon->position.y, 0.05f }) * BaseIconScaleMatrix, &ballon->LongId);
		for (int i = 0; i < 300; i++) {
			ac = generateRandomAirCraft(2, 800, 800);
			//ac->path.AddPoint(vec2(0.0f));
			AirCraftVec.push_back(ac);
			qtAc._push(ac, { ac->position.x, ac->position.y });
		}

		//86�400
		timeScale = 1.0f / 3600.0f;
	}

	void onUpdate() {
		r.RenderSelectedModel(RENDER_MODEL_HELICOPTER);
		r.RenderSelectedModel(RENDER_MODEL_GLIDER);
		r.RenderSelectedModel(RENDER_MODEL_BALLON);
		r.RenderSelectedModel(RENDER_MODEL_JET);
		r.RenderSelectedModel(RENDER_MODEL_PLANE);
		r.RenderSelectedModel(RENDER_MODEL_AIRPORT);
		r.RenderSelectedModel(RENDER_MODEL_TOWER);

		SDL_GetMouseState(&mousePos.x, &mousePos.y);
		if (qtAc._collidePoint(PointQT{ mousePos.x - 400, -1 * (mousePos.y - 400) }, 10, 10)) {
			//std::cout << "Collision" << "\n";
		}
		else {
			//std::cout << mousePos.x - 400 << "|" <<  -1 * (mousePos.y - 400) << "\n";
		}
		
		int i = 0;
		for (auto& ac : AirCraftVec) {
			br.UpdateData((BezierCurveParameters*)(ac->path.getData()), ac->path.path.size(), i);
			i += ac->path.path.size();
		}

		handleAirCraftLogic();

	}

private:
	int generateRandomValueRange(int min, int max) {
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_int_distribution<> range(min, max);
		return range(gen);
	}

	std::pair<glm::fvec2, glm::fvec2> generateRandomPath() {
		glm::fvec2 start, end;
		int idx0, idx1;

		idx0 = generateRandomValueRange(0, aircraftAmount - 1);
		idx1 = idx0;
		while (idx1 == idx0) {
			idx0 = generateRandomValueRange(0, aircraftAmount - 1);
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
		s_e = generateRandomPath();
		switch (idx)
		{
		case 0:
			ballon = new Ballon{ s_e.first, s_e.second, RENDER_MODEL_BALLON };
			r.newObject(ballon->getType(), glm::translate(glm::mat4(1.0f), glm::fvec3{ ballon->position.x, ballon->position.y, 0.05f }), &ballon->LongId);
			return (AirCraft*)ballon;
		case 1:
			jet = new Jet{ s_e.first, s_e.second, RENDER_MODEL_JET };
			r.newObject(jet->getType(), glm::translate(glm::mat4(1.0f), glm::fvec3{ jet->position.x, jet->position.y, 0.05f }), &jet->LongId);
			return (AirCraft*)jet;
		case 2:
			helicopter = new Helicopter{ s_e.first, s_e.second, RENDER_MODEL_HELICOPTER };
			r.newObject(helicopter->getType(), glm::translate(glm::mat4(1.0f), glm::fvec3{ helicopter->position.x, helicopter->position.y , 0.05f }), &helicopter->LongId);
			return (AirCraft*)helicopter;
		case 3:
			plane = new Plane{ s_e.first, s_e.second, RENDER_MODEL_PLANE };
			r.newObject(plane->getType(), glm::translate(glm::mat4(1.0f), glm::fvec3{ plane->position.x, plane->position.y, 0.05f }), &plane->LongId);
			return (AirCraft*)plane;
		case 4:
			glider = new Glider{ s_e.first, s_e.second, RENDER_MODEL_GLIDER };
			r.newObject(glider->getType(), glm::translate(glm::mat4(1.0f), glm::fvec3{ glider->position.x, glider->position.y, 0.05f }), &glider->LongId);
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
		r.newModel(idModel, vertexBuff, program, 6, GL_TRIANGLES, texture, 500);
	}

	void generateStaticObjects(int mapWidth, int mapHeight) {
		int i;
		StaticObj* st;
		glm::fvec2 position;
		for (i = 0; i < 100; i++) {
			position = { generateRandomValueRange(-mapWidth / 2.0f, mapWidth / 2.0f), generateRandomValueRange(-mapHeight / 2.0f, mapHeight / 2.0f) };
			st = new StaticObj{ position, RENDER_MODEL_AIRPORT };

			//ADD THIS TO CONFIG
			if (!qtAp._collidePoint(PointQT{ position.x, position.y }, 10, 10)) {
				AirPortsVec.push_back(st);
				qtAp._push(st, { st->position.x, st->position.y });
				r.newObject(st->getType(), glm::translate(glm::mat4(1.0f), glm::fvec3{ st->position.x, st->position.y, 0.05f }), &st->LongId);
				aircraftAmount++;
			}
		}

		for (i = 0; i < 10; i++) {
			position = { generateRandomValueRange(-mapWidth / 2.0f, mapWidth / 2.0f), generateRandomValueRange(-mapHeight / 2.0f, mapHeight / 2.0f) };
			st = new StaticObj{ position, RENDER_MODEL_TOWER };

			//ADD THIS TO CONFIG
			if (!qtT._collidePoint(PointQT{ position.x, position.y }, 25, 25)) {
				TowersVec.push_back(st);
				qtT._push(st, { st->position.x, st->position.y });
				r.newObject(st->getType(), glm::translate(glm::mat4(1.0f), glm::fvec3{ st->position.x, st->position.y, 0.05f }), &st->LongId);
			}
		}
	}

	void handleAirCraftsMovement(AirCraft*& ac, float t) {
		ac->SetAngle(ac->CalcAngle());
		ac->position = ac->path.getBezierPosition(ac->path.GetCurrentSection(), t);
		r.BindActiveModel((*(RENDER_LONG_ID*)&ac->LongId).ModelId);
		r.SetObjectMatrix((*(RENDER_LONG_ID*)&ac->LongId).ObjectId, glm::translate(glm::mat4(1.0f), glm::fvec3{ac->position.x, ac->position.y, 0.05f}) * glm::rotate(glm::mat4(1.0f), ac->angle + 1.5707963267948966f, glm::vec3(0, 0, 1)), true);
	}

	void handleAirCraftLogic() {
		float t = 0.0f;

		std::vector<AirCraft*> AirCraftsToRemove;
		for (AirCraft* ac : AirCraftVec) {
			t = 0.00003f;
			handleAirCraftsMovement(ac, t);
			if (glm::distance(ac->position, ac->path.destination) < 3.0f) {
				AirCraftsToRemove.push_back(ac);
			}
		}
		for (AirCraft* ac : AirCraftsToRemove) {
			r.DisableObjectL(ac->LongId);
			r.deleteObject((*(RENDER_LONG_ID*)&ac->LongId).ModelId, (*(RENDER_LONG_ID*)&ac->LongId).ObjectId);
			AirCraftVec.erase(std::remove(AirCraftVec.begin(), AirCraftVec.end(), ac), AirCraftVec.end());
		}

		qtAc._clear();
		for (AirCraft* ac : AirCraftVec) {
			qtAc._push(ac, { ac->position.x, ac->position.y });
		}
	}

	void rotateAirCraft(AirCraft* airCraft, glm::fvec3 destination) {
		//FOR NOW
		//float angle = 0;

		//airCraft->angle;
	}
private:
	QT<AirCraft> qtAc = { 2000, 2000 };

	std::vector<StaticObj*> AirPortsVec;
	QT<StaticObj> qtAp = { 2000, 2000 };

	std::vector<StaticObj*> TowersVec;
	QT<StaticObj> qtT = { 2000, 2000 };


	int aircraftAmount = 0;

	RenderGL r;
	VertexBuffer vertexBuff;
	Program program;

	BezierRenderer br;

	float timeScale;
};