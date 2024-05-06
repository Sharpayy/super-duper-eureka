#include <cstdio>
#include "iWinSDL.h"
#include "iRendered.h"
#include "iReaders.h"
#include <ctime>
#include "Models.h"
#include <gtc/matrix_transform.hpp>
#include <functional>
#include "Objects.h"

uint32_t ulMapScale;
uint32_t ulMapMove;
uint32_t prgmRenderMap;

uint32_t MapSizeX;
uint32_t MapSizeY;

uint32_t MapCx;
uint32_t MapCy;
uint32_t MapSs;
float MapFs;

float MapScaleFactor = 1.2f;

using namespace glm;

void SetMapScale(float scale)
{
	glUseProgram(prgmRenderMap);
	glUniform1f(ulMapScale, scale);
}

void SetMapPosition(vec2 p)
{
	glUseProgram(prgmRenderMap);
	glUniform2f(ulMapMove, p.x, p.y);
}

void SetMapPscale(float p)
{
	// mapx*s = p
	// s = p / mapx
	SetMapScale(p / MapSizeX);
}

void SetMapPposition(vec2 p)
{
	// MapX*s = 100
	// s = 100 / Mapx
	SetMapPosition(p / vec2(MapSizeX, MapSizeY));
}

#define RENDER_MODEL_SQUARE1 1

#define DEBUG_INFO_SPACE 2048
char debugInfo[DEBUG_INFO_SPACE];

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

	float vis = MapScaleFactor;

	if (e->type == SDL_KEYDOWN)
	{
		if (e->key.keysym.sym == SDLK_q)
		{
			uint32_t dsxc = MapSizeX * MapFs;
			uint32_t dsyc = MapSizeY * MapFs;

			MapFs *= vis;

			uint32_t dsxn = MapSizeX * MapFs;
			uint32_t dsyn = MapSizeY * MapFs;

			MapCx -= (dsxn - dsxc) / 2.0f;
			MapCy -= (dsyn - dsyc) / 2.0f;
		}
		if (e->key.keysym.sym == SDLK_e)
		{
			uint32_t dsxc = MapSizeX * MapFs;
			uint32_t dsyc = MapSizeY * MapFs;

			MapFs /= vis;

			uint32_t dsxn = MapSizeX * MapFs;
			uint32_t dsyn = MapSizeY * MapFs;

			MapCx += (dsxc - dsxn) / 2.0f;
			MapCy += (dsyc - dsyn) / 2.0f;
		}

		if (e->key.keysym.sym == SDLK_w)
			MapCy += 20;
		if (e->key.keysym.sym == SDLK_s)
			MapCy -= 20;
		if (e->key.keysym.sym == SDLK_d)
			MapCx += 20;
		if (e->key.keysym.sym == SDLK_a)
			MapCx -= 20;
		
		SetMapPposition(vec2(MapCx, MapCy));
		//SetMapPscale((float)MapSs);
		SetMapScale(MapFs);
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

	simpleProgram.use();
	BindSampler("image0", 0, simpleProgram.id);
	ulMapScale = glGetUniformLocation(simpleProgram.id, "MapScale");
	ulMapMove = glGetUniformLocation(simpleProgram.id, "MapMove");
	prgmRenderMap = simpleProgram.id;

	// ppcpy setup
	ppcpy.use();
	BindSampler("image0", 0, ppcpy.id);

	prgm.use();
	BindSampler("image0", 0, prgm.id);

	popaque.use();
	BindSampler("image0", 0, popaque.id);

	Buffer<GL_ARRAY_BUFFER> SquareVBO = Buffer<GL_ARRAY_BUFFER>(sizeof(Square11VertexUv_2d), Square11VertexUv_2d, GL_STATIC_DRAW);
	Buffer<GL_ELEMENT_ARRAY_BUFFER> SquareEBO = Buffer<GL_ELEMENT_ARRAY_BUFFER>(sizeof(Square1Indice_2d), Square1Indice_2d, GL_STATIC_DRAW);

	VertexBuffer Square = VertexBuffer();
	Square.bind();
	SquareVBO.bind();
	SquareEBO.bind();

	Square.addAttrib(GL_FLOAT, 0, 2, sizeof(float) * 4, 0);
	Square.addAttrib(GL_FLOAT, 1, 2, sizeof(float) * 4, 8);
	Square.enableAttrib(0);
	Square.enableAttrib(1);

	Buffer<GL_ARRAY_BUFFER> ppcpy_planeA = Buffer<GL_ARRAY_BUFFER>(sizeof(Plane1x1VertexArrayTriangleUv), Plane1x1VertexArrayTriangleUv, GL_STATIC_DRAW);
	VertexBuffer ppcpy_planeVB = VertexBuffer();
	ppcpy_planeVB.bind();
	ppcpy_planeA.bind();
	ppcpy_planeVB.addAttrib(GL_FLOAT, 0, 3, 20, 0);
	ppcpy_planeVB.addAttrib(GL_FLOAT, 1, 2, 20, 12);
	ppcpy_planeVB.enableAttrib(0);
	ppcpy_planeVB.enableAttrib(1);

	

	RenderGL r = RenderGL(100);
	r.setCameraMatrix(glm::mat4(1.0f));
	r.setProjectionMatrix(glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 100.0f));
	r.UpdateShaderData();

	int x, y, c;
	uint8_t* MapTextureData = (uint8_t*)LoadImageData("ukMap.png", 1, &c, &x, &y);
	Texture2D MapTexture = Texture2D(MapTextureData, x, y, GL_RGBA, GL_RGBA);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	MapTexture.genMipmap();

	MapSizeX = x;
	MapSizeY = y;
	MapCx = 0;
	MapCy = 0;
	MapSs = 1000;
	MapFs = 1.0f;

	FreeImageData(MapTextureData);

	r.newModel(RENDER_MODEL_SQUARE1, Square, simpleProgram, 6, GL_TRIANGLES, MapTexture, 50);


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
	clock_t evLoopTimeTarget = 1000;

	glEnable(GL_DEPTH_TEST);
	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);

	mat4 CameraMatrix = lookAt(vec3(0.0f, 0.0f, 1.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
	r.setCameraMatrix(CameraMatrix);
	r.UpdateShaderDataCamera();
	
	SetMapScale(1.0f);
	SetMapPosition(vec2(0.0f));

	CPUPerformanceTimer LoopElapsedTime = CPUPerformanceTimer();
	PerformanceTimer RenderElapsedTime = PerformanceTimer();
	RenderElapsedTime.Reset();
	LoopElapsedTime.Reset();
	uint32_t lp = 0;

	int64_t SumRenderTime = 0;
	while (true)
	{
		evLoopStart = clock();
		LoopElapsedTime.Start();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		

		RenderElapsedTime.TimeStart();
		r.RenderSelectedModel(RENDER_MODEL_SQUARE1);
		RenderElapsedTime.TimeEnd();
		
		win.swap();
		win.handleEvents();

		lp++;
		LoopElapsedTime.End();
		evCurrTime += clock() - evLoopStart;
		SumRenderTime += RenderElapsedTime.GetElapsedTime();
		if (evCurrTime >= evLoopTimeTarget)
		{
			
			char NewWinTitle[64];
			SumRenderTime = SumRenderTime / 1000000;
			
			snprintf(NewWinTitle, 64, "Fps: %d Rendr: %fms", lp, (float)SumRenderTime / (float)lp);

			SDL_SetWindowTitle(win.win, NewWinTitle);
			evCurrTime -= evLoopTimeTarget;
			lp = 0;
			SumRenderTime = 0;
		}

	}

}