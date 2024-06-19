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
	AManager(RenderGL* r, Buffer<GL_ARRAY_BUFFER> vbo, Program program, Program mapProgram, BezierRenderer* br, Buffer<GL_ELEMENT_ARRAY_BUFFER> ebo, Camera* camera, Program m, Config& cfg);

	void onUpdate(float dt);

private:
	int generateRandomValueRange(int min, int max);

	std::pair<StaticObj*, StaticObj*> generateRandomPath(float s);

	AirCraft* generateRandomAirCraft(int idx, int mapWidth, int mapHeight);

	void addModel(Texture2D texture, uint8_t idModel);

	void generateStaticObjects(int mapWidth, int mapHeight);

	void handleAirCraftsMovement(AirCraft*& ac, float dt);
	void handleAirCraftLogic(float t);

	void handleAirCraftCollision(AirCraft*& ac, float w, float h, std::vector<AirCraft*>& AircraftsToRemove);

	float getAirCraftAngle(AirCraft* ac);

	bool keyPressedOnce(SDL_Scancode key);

private:
	int calculateBestObjectAmount(float factor, float def);

	bool elementExist(uint64_t id);

	std::unordered_map<uint8_t, std::vector<AirCraft*>> AirCraftMap;
	std::unordered_map<uint64_t, AirCraft*> wMap;

	AirCraft* selectedAircraft;

	QT<AirCraft>* qtAc;

	std::vector<StaticObj*> AirPortsVec;
	QT<StaticObj>* qtAp;

	std::vector<StaticObj*> TowersVec;
	QT<StaticObj>* qtT;

	RenderGL* r;
	Buffer<GL_ARRAY_BUFFER> vbo;
	Buffer<GL_ELEMENT_ARRAY_BUFFER> ebo;
	Program program;

	Map map;
	Camera* camera;

	BezierRenderer* br;
	CollisionDrawer cd;
	uint32_t pathRenderCount;
	uint32_t ls;

	struct _generator {
		std::random_device rd;
		std::mt19937 gen;
	} generator;

	struct aircraftAmountData {
		int amount;
		float amin;
		float amax;
		//For not mapping it all the time
		float chance;
	} acData;

	acHeightData hdata;

	Config* cfg;

	AircraftRenderData ard;
};