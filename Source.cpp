#include <cstdio>
#include "iWinSDL.h"
#include "iRendered.h"
#include "iReaders.h"
#include <ctime>
#include "Models.h"
#include <gtc/matrix_transform.hpp>
#include <functional>
#include "Objects.h"

#define RENDER_MODEL_SQUARE1 1

#define DEBUG_INFO_SPACE 2048
char debugInfo[DEBUG_INFO_SPACE];
using namespace glm;

class Camera
{
public:
	Camera(vec3 pos)
	{
		yaw = 90.0f; pitch = 0.0f; roll = 0.0f;
		this->pos = pos;

		f = normalize(vec3(
			cos(yaw) * cos(pitch),
			sin(pitch),
			cos(pitch) * sin(yaw)
		));
		r = normalize(cross(f, vec3(0.0f, 1.0f, 0.0f)));
		u = normalize(cross(f, r));
	}
	mat4 getMatrix()
	{
		f = normalize(vec3(
			cos(yaw) * cos(pitch),
			sin(pitch),
			cos(pitch) * sin(yaw)
		));

		mat4 roll_mat = glm::rotate(mat4(1.0f), roll, f);

		r = normalize(cross(f, vec3(0.0f, 1.0f, 0.0f)));
		u = mat3(roll_mat) * normalize(cross(f, r));
		mat4 M = lookAt(pos, f + pos, u);
		return M;
	}
	vec3 getPos()
	{
		return pos;
	}
	vec3* getPPos()
	{
		return &pos;
	}
	void addYaw(float x)
	{
		yaw += x;
	}
	void addPitch(float x)
	{
		pitch += x;
	}
	void addRoll(float x)
	{
		roll += x;
	}
	void setMatrix(mat4* matrix)
	{
		f = normalize(vec3(
			cos(yaw) * cos(pitch),
			sin(pitch),
			cos(pitch) * sin(yaw)
		));

		mat4 roll_mat = glm::rotate(mat4(1.0f), roll, f);

		r = normalize(cross(f, vec3(0.0f, 1.0f, 0.0f)));
		u = mat3(roll_mat) * normalize(cross(f, r));

		*matrix = lookAt(pos, f + pos, u);
	}
	void setYaw(float yaw)
	{
		this->yaw = yaw;
	}
	void setPitch(float pitch)
	{
		this->pitch = pitch;
	}
	void setRoll(float roll)
	{
		this->roll = roll;
	}
	vec3 getRight()
	{
		return r;
	}
	void setPos(vec3 pos)
	{
		this->pos = pos;
	}
	void addPos(vec3 pos)
	{
		this->pos += pos;
	}
	void posAddFront(float s)
	{
		pos += f * s;
	}
	void posAddRight(float s)
	{
		pos += r * s;
	}

	float yaw, pitch, roll;
	vec3 pos;
	vec3 r, u, f;
};

void CustomEventDispatcher(SDL_Event* e, Camera* cam)
{
	float pd = 0.05f, yd = 0.05f;
	float ms = 0.1f;
	if (e->type == SDL_QUIT)
		exit(0);

	if (e->type == SDL_KEYDOWN)
	{
		if (e->key.keysym.sym == SDLK_UP)
			cam->addPitch(-pd);
		if (e->key.keysym.sym == SDLK_DOWN)
			cam->addPitch(pd);
		if (e->key.keysym.sym == SDLK_RIGHT)
			cam->addYaw(-yd);
		if (e->key.keysym.sym == SDLK_LEFT)
			cam->addYaw(yd);

		if (e->key.keysym.sym == SDLK_w)
			cam->posAddFront(ms);
		if (e->key.keysym.sym == SDLK_s)
			cam->posAddFront(-ms);
		if (e->key.keysym.sym == SDLK_d)
			cam->posAddRight(-ms);
		if (e->key.keysym.sym == SDLK_a)
			cam->posAddRight(ms);
	}
}

int main(int argc, char** argv)
{
	WindowSDL win = WindowSDL("Test", 800, 800, SDL_WINDOW_SHOWN);
	win.glCreateContext();
	iinit();


	const char* srcShaderVertex = GetFileData((char*)"shaderVertex.glsl")->c_str();
	const char* srcShaderFragment = GetFileData((char*)"shaderFragment.glsl")->c_str();
	const char* srcShaderCompute = GetFileData((char*)"TransparentResolveCompute.glsl")->c_str();
	const char* srcPstPrVertex = GetFileData((char*)"VertexPostProcessing.glsl")->c_str();
	const char* srcPstPrFrag = GetFileData((char*)"FragmentPostProcessingCpy.glsl")->c_str();
	const char* srcOpaqueFrag = GetFileData((char*)"OpaqueFragmentShader.glsl")->c_str();
	const char* srcZeroAtomic = GetFileData((char*)"AtomicZeroFeedback.glsl")->c_str();
	const char* srcShaderOncColFrag = GetFileData((char*)"ShaderFragmentOneColor.glsl")->c_str();

	Shader<GL_VERTEX_SHADER>   shdrVertex   = Shader<GL_VERTEX_SHADER>(srcShaderVertex);
	Shader<GL_FRAGMENT_SHADER> shdrFragment = Shader<GL_FRAGMENT_SHADER>(srcShaderFragment);
	Shader<GL_COMPUTE_SHADER>  shdrCompute = Shader<GL_COMPUTE_SHADER>(srcShaderCompute);
	Shader<GL_VERTEX_SHADER>   shdrPstPrVertex = Shader<GL_VERTEX_SHADER>(srcPstPrVertex);
	Shader<GL_FRAGMENT_SHADER> shdrPstPrFrag = Shader<GL_FRAGMENT_SHADER>(srcPstPrFrag);
	Shader<GL_FRAGMENT_SHADER> shdrOpaqueFrag = Shader<GL_FRAGMENT_SHADER>(srcOpaqueFrag);
	Shader<GL_COMPUTE_SHADER>  shdrCompSwapAtc = Shader<GL_COMPUTE_SHADER>(srcZeroAtomic);
	Shader<GL_FRAGMENT_SHADER> shdrFrgOneColor = Shader<GL_FRAGMENT_SHADER>(srcShaderOncColFrag);

	Program simpleProgram = Program();
	simpleProgram.programAddShader(shdrFrgOneColor.id);
	simpleProgram.programAddShader(shdrVertex.id);
	simpleProgram.programCompile();

	Program swapAtomicCnt = Program();
	swapAtomicCnt.programAddShader(shdrCompSwapAtc.id);
	swapAtomicCnt.programCompile();
	swapAtomicCnt.programGetDebugInfo(debugInfo, DEBUG_INFO_SPACE);
	printf("%s\n", debugInfo);

	Program prgm = Program();
	prgm.programAddShader(shdrFragment.id);
	prgm.programAddShader(shdrVertex.id);
	prgm.programCompile();

	Program prgmc = Program();
	prgmc.programAddShader(shdrCompute.id);
	prgmc.programCompile();

	Program ppcpy = Program();
	ppcpy.programAddShader(shdrPstPrFrag.id);
	ppcpy.programAddShader(shdrPstPrVertex.id);
	ppcpy.programCompile();

	Program popaque = Program();
	popaque.programAddShader(shdrOpaqueFrag.id);
	popaque.programAddShader(shdrVertex.id);
	popaque.programCompile();

	//if (prgm.programGetDebugInfo(debugInfo, DEBUG_INFO_SPACE) == GL_FALSE)
	//	printf("%s\n", debugInfo);

	prgmc.programGetDebugInfo(debugInfo, DEBUG_INFO_SPACE);
	printf("%s\n", debugInfo);

	// ppcpy setup
	ppcpy.use();
	BindSampler("image0", 0, ppcpy.id);

	prgm.use();
	BindSampler("image0", 0, prgm.id);

	popaque.use();
	BindSampler("image0", 0, popaque.id);


	float vertexModel[] = 
	{
	-1.0, -1.0,  1.0, 0.0f, 0.0f,
	 1.0, -1.0,  1.0, 1.0f, 0.0f, 
	 1.0,  1.0,  1.0, 1.0f, 1.0f,
	-1.0,  1.0,  1.0, 0.0f, 1.0f,
	// back
	-1.0, -1.0, -1.0, 0.0f, 0.0f,
	 1.0, -1.0, -1.0, 1.0f, 0.0f,
	 1.0,  1.0, -1.0, 1.0f, 1.0f,
	-1.0,  1.0, -1.0, 0.0f, 1.0f
	};

	uint32_t indiceModel[] =
	{
		// front colors
		0, 1, 2,
		2, 3, 0,
		// right
		1, 5, 6,
		6, 2, 1,
		// back
		7, 6, 5,
		5, 4, 7,
		// left
		4, 0, 3,
		3, 7, 4,
		// bottom
		4, 5, 1,
		1, 0, 4,
		// top
		3, 2, 6,
		6, 7, 3
	};
	
	Buffer<GL_ARRAY_BUFFER> SquareVBO = Buffer<GL_ARRAY_BUFFER>(sizeof(Square1Vertex_2d), Square1Vertex_2d, GL_STATIC_DRAW);
	Buffer<GL_ELEMENT_ARRAY_BUFFER> SquareEBO = Buffer<GL_ELEMENT_ARRAY_BUFFER>(sizeof(Square1Indice_2d), Square1Indice_2d, GL_STATIC_DRAW);

	VertexBuffer Square = VertexBuffer();
	Square.bind();
	SquareVBO.bind();
	SquareEBO.bind();

	Square.addAttrib(GL_FLOAT, 0, 2, sizeof(float) * 2, 0);
	Square.enableAttrib(0);

	Buffer<GL_ARRAY_BUFFER> ppcpy_planeA = Buffer<GL_ARRAY_BUFFER>(sizeof(Plane1x1VertexArrayTriangleUv), Plane1x1VertexArrayTriangleUv, GL_STATIC_DRAW);
	VertexBuffer ppcpy_planeVB = VertexBuffer();
	ppcpy_planeVB.bind();
	ppcpy_planeA.bind();
	ppcpy_planeVB.addAttrib(GL_FLOAT, 0, 3, 20, 0);
	ppcpy_planeVB.addAttrib(GL_FLOAT, 1, 2, 20, 12);
	ppcpy_planeVB.enableAttrib(0);
	ppcpy_planeVB.enableAttrib(1);


	Buffer<GL_ARRAY_BUFFER> vbo = Buffer<GL_ARRAY_BUFFER>(sizeof(vertexModel), vertexModel, GL_STATIC_DRAW);
	Buffer<GL_ELEMENT_ARRAY_BUFFER> ebo = Buffer<GL_ELEMENT_ARRAY_BUFFER>(sizeof(indiceModel), indiceModel, GL_STATIC_DRAW);

	VertexBuffer vao = VertexBuffer();
	vao.bind();
	vbo.bind();
	ebo.bind();
	vao.addAttrib(GL_FLOAT, 0, 3, 20, 0);
	vao.enableAttrib(0);
	vao.addAttrib(GL_FLOAT, 1, 2, 20, 12);
	vao.enableAttrib(1);
	glBindVertexArray(0);
	

	RenderGL r = RenderGL(100);
	r.setCameraMatrix(glm::mat4(1.0f));
	r.setProjectionMatrix(glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 100.0f));
	r.UpdateShaderData();

	r.newModel(RENDER_MODEL_SQUARE1, Square, simpleProgram, 6, GL_TRIANGLES, Texture2D(), 50);


	Camera cam = Camera(vec3(0.0f, 0.0f, 0.0f));
	r.setCameraMatrix(cam.getMatrix());
	r.UpdateShaderDataCamera();

	win.customEventDispatch = std::bind(CustomEventDispatcher, std::placeholders::_1, &cam);

	uint64_t sqr0, sqr1;
	
#define OM(A) r.GetObjectMatrix(A)

	r.newObject(RENDER_MODEL_SQUARE1, mat4(1.0f), &sqr0);


	glm::mat4 mt = glm::mat4(1.0f);

	clock_t evLoopStart = 0;
	clock_t evCurrTime = 0;
	clock_t evLoopTimeTarget = 500;

	uint32_t objectList[10];

	// A-Buffer
	uint32_t z = 0;
	uint32_t zd = 0;
	uint32_t st = 0;

	glEnable(GL_DEPTH_TEST);
	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	while (true)
	{
		evLoopStart = clock();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		
		
		r.setCameraMatrix(cam.getMatrix());
		r.UpdateShaderDataCamera();

		r.RenderSelectedModel(RENDER_MODEL_SQUARE1);
		
		win.swap();
		win.handleEvents();

		evCurrTime += clock() - evLoopStart;
		if (evCurrTime >= evLoopTimeTarget)
		{
			if (z > 5)
			{
				z = 0;
				st = 1;
			}
			if (zd > 5)
			{
				zd = 0;
				st = 0;
			}
			if (st == 0)
			{
				objectList[z] = r.newObject(RENDER_MODEL_SQUARE1, translate(mat4(1.0f), vec3(0.0f, 0.0f, -0.5f * z)));
				z++;
			}
			else
			{
				r.DisableObject(objectList[zd]);
				r.deleteObject(RENDER_MODEL_SQUARE1, objectList[zd]);
				zd++;
			}
			evCurrTime -= evLoopTimeTarget;
		}

	}

}