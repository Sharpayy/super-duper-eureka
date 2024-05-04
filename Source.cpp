#include <cstdio>
#include "iWinSDL.h"
#include "iRendered.h"
#include "iReaders.h"
#include <ctime>
#include "Models.h"
#include <gtc/matrix_transform.hpp>
#include <functional>

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

	Shader<GL_VERTEX_SHADER>   shdrVertex   = Shader<GL_VERTEX_SHADER>(srcShaderVertex);
	Shader<GL_FRAGMENT_SHADER> shdrFragment = Shader<GL_FRAGMENT_SHADER>(srcShaderFragment);
	Shader<GL_COMPUTE_SHADER>  shdrCompute = Shader<GL_COMPUTE_SHADER>(srcShaderCompute);
	Shader<GL_VERTEX_SHADER>   shdrPstPrVertex = Shader<GL_VERTEX_SHADER>(srcPstPrVertex);
	Shader<GL_FRAGMENT_SHADER> shdrPstPrFrag = Shader<GL_FRAGMENT_SHADER>(srcPstPrFrag);
	Shader<GL_FRAGMENT_SHADER> shdrOpaqueFrag = Shader<GL_FRAGMENT_SHADER>(srcOpaqueFrag);
	Shader<GL_COMPUTE_SHADER>  shdrCompSwapAtc = Shader<GL_COMPUTE_SHADER>(srcZeroAtomic);

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


	int red_x, red_y, red_chn;
	void* red_data = LoadImageData("red.png", false, &red_chn, &red_x, &red_y);

	int green_x, green_y, green_chn;
	void* green_data = LoadImageData("green.png", false, &green_chn, &green_x, &green_y);

	int blue_x, blue_y, blue_chn;
	void* blue_data = LoadImageData("blue.png", false, &blue_chn, &blue_x, &blue_y);

	int vi_x, vi_y, vi_chn;
	void* vi_data = LoadImageData("vi.png", false, &vi_chn, &vi_x, &vi_y);

	int szr_x, szr_y, szr_chn;
	void* szr_data = LoadImageData("szr.png", false, &szr_chn, &szr_x, &szr_y);

	Texture2D green = Texture2D(green_data, green_x, green_y, GL_RGBA, GL_RGBA);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	green.genMipmap();
	Texture2D red = Texture2D(red_data, red_x, red_y, GL_RGBA, GL_RGBA);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	red.genMipmap();
	Texture2D blue = Texture2D(blue_data, blue_x, blue_y, GL_RGBA, GL_RGBA);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	blue.genMipmap();
	Texture2D vi = Texture2D(vi_data, vi_x, vi_y, GL_RGBA, GL_RGBA);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	vi.genMipmap();
	Texture2D szr = Texture2D(szr_data, szr_x, szr_y, GL_RGBA, GL_RGBA);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	szr.genMipmap();

	r.newModel(1, vao, popaque, 36, GL_TRIANGLES, blue, 60000); // 5000
	r.newModel(2, vao, popaque, 36, GL_TRIANGLES, green, 50);
	r.newModel(3, vao, popaque, 36, GL_TRIANGLES, red, 60);
	r.newModel(5, vao, popaque, 36, GL_TRIANGLES, vi, 60);
	r.newModel(4, vao, popaque, 1, GL_POINTS, Texture2D(), 500);
	r.newModel(10, vao, popaque, 36, GL_TRIANGLES, szr, 50);

	Camera cam = Camera(vec3(0.0f, 0.0f, 0.0f));
	r.setCameraMatrix(cam.getMatrix());
	r.UpdateShaderDataCamera();

	win.customEventDispatch = std::bind(CustomEventDispatcher, std::placeholders::_1, &cam);

	uint64_t obj0, obj1;
	
#define OM(A) r.GetObjectMatrix(A)

	RENDER_OBJECT_ID t0 = r.newObject(3, glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f)));
	RENDER_OBJECT_ID t1 = r.newObject(3, glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -2.0f)));

	r.SetObjectMatrix(t0, glm::rotate(OM(t0), 0.2f, glm::vec3(1, 1, 0)));
	r.SetObjectMatrix(t1, glm::rotate(OM(t1), 0.9f, glm::vec3(0, 1, 1)));

	RENDER_OBJECT_ID t2 = r.newObject(2, glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.3f, -5.0f)), &obj0);
	r.SetObjectMatrix(t2, glm::rotate(OM(t2), 0.5f, glm::vec3(1, 1, 0)));

	RENDER_OBJECT_ID t3 = r.newObject(1, glm::translate(glm::mat4(1.0f), glm::vec3(1.5f, 1.3f, -14.0f)), &obj1);
	r.SetObjectMatrix(t2, glm::rotate(OM(t2), 2.2f, glm::vec3(1, 0, 1)));

	for (float i = 0.0f; i < 35.0f; i += 5.0f)
		r.newObject(1, glm::translate(glm::mat4(1.0f), glm::vec3(-20.5f + i, 2.0f, -10.0f)));
	
	r.newObject(4, glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -3.0f)));
	r.newObject(4, glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -3.1f)));

	RENDER_OBJECT_ID v0 = r.newObject(5, glm::translate(glm::mat4(1.0f), glm::vec3(0.2f, 0.9f, -8.0f)));
	r.SetObjectMatrix(v0, glm::rotate(OM(v0), 0.4f, glm::vec3(0, 1, 1)));

	RENDER_OBJECT_ID sz0 = r.newObject(10, glm::translate(glm::mat4(1.0f), glm::vec3(0.5f, 0.4f, 1.9f)));
	r.SetObjectMatrix(sz0, glm::rotate(OM(sz0), 2.2f, glm::vec3(0, 0, 1)));
	RENDER_OBJECT_ID sz1 = r.newObject(10, glm::translate(glm::mat4(1.0f), glm::vec3(-0.2f, 0.4f, -4.9f)));
	r.SetObjectMatrix(sz1, glm::rotate(OM(sz1), 1.35f, glm::vec3(1, 0, 1)));

	glm::mat4 mt = glm::mat4(1.0f);

	LoopStatistics LS = LoopStatistics();
	CPUPerformanceTimer CPU_T = CPUPerformanceTimer();
	uint32_t fps = 0;

	clock_t time_s = clock();

	RENDER_OBJECT_ID fo = r.newObject(1, glm::mat4(1.0f));
	for (int i = 0; i < 50000; i++)
		r.newObject(1, glm::mat4(1.0f));

	clock_t time_a = clock();

	for (int i = 0; i < 50000; i++)
		r.DisableObject(fo + i);

	clock_t time_e = clock();
	glFinish();
	printf("Disable: %d\nCreate: %d\n", time_e - time_a, time_a - time_s);


	// A-Buffer
	float z = 0;
	glEnable(GL_DEPTH_TEST);
	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	while (true)
	{
		CPU_T.Start();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
		//atmLdk.zero();
		
		
		r.setCameraMatrix(cam.getMatrix());
		r.UpdateShaderDataCamera();

		r.RenderSelectedModel(10);

		r.RenderSelectedModel(2);
		r.RenderSelectedModel(1);
		r.RenderSelectedModel(5);
		r.RenderSelectedModel(3);
		
		win.swap();
		win.handleEvents();


		CPU_T.End();
		fps = CalcFps(CPU_T, &LS);
		if (fps != 0)
		{
			//atmLdk.sync(ATC_SYNC_SHADER);
			r.BindActiveModel(5);
			if (r.IsObjectActive(v0) == true)
			{
				r.DisableObject(v0);
				r.DisableObjectL(obj0);
				r.DisableObjectL(obj1);
			}
			else
			{
				r.EnableObject(v0);
				r.EnableObjectL(obj0);
				r.EnableObjectL(obj1);
			}
		}
		//SDL_Delay(16);


	}

}