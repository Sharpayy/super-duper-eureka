#include "Bezier.h"
#include "octree.h"

#ifdef _DEBUG
#define AM_ASSERT(A) assert(A)
#else
#define AM_ASSERT(A)
#endif

extern glm::mat4 BaseIconScaleMatrix;

SDL_Point mousePos;

class AManager {
public:
	std::vector<AirCraft*> airCraftVec;
	AManager() = default;
	AManager(RenderGL& r, VertexBuffer& vertexBuff, Program program) {
		this->r = r;
		this->vertexBuff = vertexBuff;
		this->program = program;
		m_alpha = 0.002f;

		int x, y, c;
		Texture2D MapTexture;
		uint8_t* MapTextureData;

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

		qt._alloc(8);
		AirCraft* ac;
		//Ballon* ballon = new Ballon{ {0,0}, {0,0} };
		//r.newObject(RENDER_MODEL_BALLON, glm::translate(glm::mat4(1.0f), glm::fvec3{ ballon->position.x, ballon->position.y, 0.05f }) * BaseIconScaleMatrix, &ballon->LongId);
		for (int i = 0; i < 5000; i++) {
			ac = generateRandomAirCraft(1, 400, 400);
			airCraftVec.push_back(ac);
			qt._push(ac, { ac->position.x, ac->position.y});
		}

		//86 400
		timeScale = 1.0f / 3600.0f;
	}

	void onUpdate() {
		r.RenderSelectedModel(RENDER_MODEL_HELICOPTER);
		r.RenderSelectedModel(RENDER_MODEL_GLIDER);
		r.RenderSelectedModel(RENDER_MODEL_BALLON);
		r.RenderSelectedModel(RENDER_MODEL_JET);
		r.RenderSelectedModel(RENDER_MODEL_PLANE);
		
		SDL_GetMouseState(&mousePos.x, &mousePos.y);
		if (qt._collidePoint(PointQT{ mousePos.x - 400, -1 * (mousePos.y - 400) }, 10, 10)) {
			//std::cout << "Collision" << "\n";
		}
		else {
			//std::cout << mousePos.x - 400 << "|" <<  -1 * (mousePos.y - 400) << "\n";
		}
		
	}

private:
	std::pair<glm::fvec2, glm::fvec2> generateRandomPath(int mapWidth, float mapHeight) {
		glm::fvec2 start, end;
		float x, y;

		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_int_distribution<> distrW(-mapWidth / 2.0f, mapWidth / 2.0f);
		std::uniform_int_distribution<> distrH(-mapHeight / 2.0f, mapHeight / 2.0f);

		x = distrW(gen);
		y = distrH(gen);
		start = { x - 400, y - 400 };
		x = distrW(gen);
		y = distrH(gen);
		end = { x, y };
		return { start, end };
	}

	AirCraft* generateRandomAirCraft(int idx, int mapWidth, int mapHeight) {
		std::pair<glm::fvec2, glm::fvec2> s_e;

		s_e = generateRandomPath(mapHeight, mapHeight);
		m_alpha += 0.005f;

		Glider* glider;
		Jet* jet;
		Ballon* ballon;
		Helicopter* helicopter;
		Plane* plane;
		switch (idx)
		{
		case 0:
			ballon = new Ballon{ s_e.first, s_e.second };
			r.newObject(RENDER_MODEL_BALLON, glm::translate(glm::mat4(1.0f), glm::fvec3{ ballon->position.x, ballon->position.y, m_alpha }), &ballon->LongId);
			return (AirCraft*)ballon;
		case 1:
			jet = new Jet{ s_e.first, s_e.second };
			r.newObject(RENDER_MODEL_JET, glm::translate(glm::mat4(1.0f), glm::fvec3{jet->position.x, jet->position.y, m_alpha }), &jet->LongId);
			return (AirCraft*)jet;
		case 2:
			 helicopter = new Helicopter{ s_e.first, s_e.second };
			r.newObject(RENDER_MODEL_HELICOPTER, glm::translate(glm::mat4(1.0f), glm::fvec3{ helicopter->position.x, helicopter->position.y , m_alpha }), &helicopter->LongId);
			return (AirCraft*)helicopter;
		case 3:
			plane = new Plane{ s_e.first, s_e.second };
			r.newObject(RENDER_MODEL_PLANE, glm::translate(glm::mat4(1.0f), glm::fvec3{ plane->position.x, plane->position.y, m_alpha }), &plane->LongId);
			return (AirCraft*)plane;
		case 4:
			glider = new Glider{ s_e.first, s_e.second };
			r.newObject(RENDER_MODEL_GLIDER, glm::translate(glm::mat4(1.0f), glm::fvec3{ glider->position.x, glider->position.y, m_alpha }), &glider->LongId);
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
		r.newModel(idModel, vertexBuff, program, 6, GL_TRIANGLES, texture, 200000);
	}

	void rotateAirCraft(AirCraft* airCraft, glm::fvec3 destination) {
		//FOR NOW
		//float angle = 0;

		//airCraft->angle;
	}
	
private:
	QT<AirCraft> qt = {11000, 11000};

	RenderGL r;
	VertexBuffer vertexBuff;
	Program program;

	float m_alpha;

	float timeScale;
};