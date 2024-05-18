#include "AManager.h"
#include "Models.h"
#include "Objects.h"

using namespace glm;
#define LOOK_DIRECTION vec3(0.0f, 0.0f, -1.0f)

typedef struct _RenderableMapSettings
{
	float MoveX;
	float MoveY;
	float ScaleX;
	float ScaleY;

	mat4 CameraMatrix;
	mat4 ScaleMatrix;
	uint32_t NeedUpdate;

} RenderableMapSettings;

// settings

uint32_t BaseIconScale = 10.0f;
float CurrIconScale = 1.0f;
mat4 BaseIconScaleMatrix = scale(mat4(1.0f), vec3(BaseIconScale));

float scale_factor = 1.2f;
float move_factor = 20.0f;

// settings


uint32_t ScreenWidth = 800;
uint32_t ScreenHeigth = 800;

#define RENDER_MODEL_SQUARE1 7

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

void CustomEventDispatcher(SDL_Event* e, RenderableMapSettings* MapSettings)
{
	if (e->type == SDL_QUIT)
		exit(0);

	if (e->type == SDL_KEYDOWN)
	{
		if (e->key.keysym.sym == SDLK_e)
		{
			MapSettings->ScaleX /= scale_factor;
			MapSettings->ScaleY /= scale_factor;
			CurrIconScale /= 1.08;
		}
		if (e->key.keysym.sym == SDLK_q)
		{
			MapSettings->ScaleX *= scale_factor;
			MapSettings->ScaleY *= scale_factor;
			CurrIconScale *= 1.08;
		}

		MapSettings->ScaleMatrix = glm::ortho(-MapSettings->ScaleX, MapSettings->ScaleX, -MapSettings->ScaleY, MapSettings->ScaleY, -1000.0f, 1000.0f);

		if (e->key.keysym.sym == SDLK_w)
			MapSettings->MoveY += move_factor;
		if (e->key.keysym.sym == SDLK_s)
			MapSettings->MoveY -= move_factor;
		if (e->key.keysym.sym == SDLK_d)
			MapSettings->MoveX += move_factor;
		if (e->key.keysym.sym == SDLK_a)
			MapSettings->MoveX -= move_factor;

		MapSettings->CameraMatrix = lookAt(vec3(MapSettings->MoveX, MapSettings->MoveY, 0.0f), vec3(MapSettings->MoveX, MapSettings->MoveY, 0.0f) + LOOK_DIRECTION, vec3(0.0f, 1.0f, 0.0f));
		MapSettings->NeedUpdate = 1;
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
	const char* srcShaderIconVertex = GetFileData((char*)"ShaderVertexIcon.glsl")->c_str();
	const char* srcShaderIconFragment = GetFileData((char*)"ShaderFragmentIcon.glsl")->c_str();

	const char* srcShaderBezierVtx = GetFileData((char*)"vtx_shader_bezier.glsl")->c_str();
	const char* srcShaderBezierTcs = GetFileData((char*)"tcs_shader_bezier.glsl")->c_str();
	const char* srcShaderBezierTes = GetFileData((char*)"tes_shader_bezier.glsl")->c_str();
	const char* srcShaderBezierFrg = GetFileData((char*)"frg_shader_bezier.glsl")->c_str();

	Shader<GL_VERTEX_SHADER>				 shdrBezVtx = Shader<GL_VERTEX_SHADER>(srcShaderBezierVtx);
	Shader<GL_TESS_CONTROL_SHADER>			 shdrBezTcs = Shader<GL_TESS_CONTROL_SHADER>(srcShaderBezierTcs);
	Shader<GL_TESS_EVALUATION_SHADER>		 shdrBezTes = Shader<GL_TESS_EVALUATION_SHADER>(srcShaderBezierTes);
	Shader<GL_FRAGMENT_SHADER>				 shdrBezFrg = Shader<GL_FRAGMENT_SHADER>(srcShaderBezierFrg);

	Shader<GL_VERTEX_SHADER>   shdrVertex = Shader<GL_VERTEX_SHADER>(srcShaderVertex);
	Shader<GL_FRAGMENT_SHADER> shdrFragment = Shader<GL_FRAGMENT_SHADER>(srcShaderFragment);
	Shader<GL_COMPUTE_SHADER>  shdrCompute = Shader<GL_COMPUTE_SHADER>(srcShaderCompute);
	Shader<GL_VERTEX_SHADER>   shdrPstPrVertex = Shader<GL_VERTEX_SHADER>(srcPstPrVertex);
	Shader<GL_FRAGMENT_SHADER> shdrPstPrFrag = Shader<GL_FRAGMENT_SHADER>(srcPstPrFrag);
	Shader<GL_FRAGMENT_SHADER> shdrOpaqueFrag = Shader<GL_FRAGMENT_SHADER>(srcOpaqueFrag);
	Shader<GL_COMPUTE_SHADER>  shdrCompSwapAtc = Shader<GL_COMPUTE_SHADER>(srcZeroAtomic);
	Shader<GL_FRAGMENT_SHADER> shdrFrgOneColor = Shader<GL_FRAGMENT_SHADER>(srcShaderOncColFrag);
	Shader<GL_FRAGMENT_SHADER> shdrFrgIcon = Shader<GL_FRAGMENT_SHADER>(srcShaderIconFragment);
	Shader<GL_VERTEX_SHADER>   shdrVertIcon = Shader<GL_VERTEX_SHADER>(srcShaderIconVertex);

	Program BezierProg = Program();
	BezierProg.programAddShader(shdrBezVtx.id);
	BezierProg.programAddShader(shdrBezTes.id);
	BezierProg.programAddShader(shdrBezTcs.id);
	BezierProg.programAddShader(shdrBezFrg.id);
	BezierProg.programCompile();

	Program iconProgram = Program();
	iconProgram.programAddShader(shdrFrgIcon.id);
	iconProgram.programAddShader(shdrVertIcon.id);
	iconProgram.programCompile();

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

	iconProgram.use();
	BindSampler("image0", 0, iconProgram.id);
	uint32_t ulIconScale = glGetUniformLocation(iconProgram.id, "uIconScale");
	glUniform1f(ulIconScale, 1.0f);

	simpleProgram.use();
	BindSampler("image0", 0, simpleProgram.id);

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

	glViewport(0, 0, ScreenWidth, ScreenHeigth);

	RenderGL r = RenderGL(100);
	r.setCameraMatrix(glm::mat4(1.0f));
	//r.setProjectionMatrix(glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 100.0f));
	r.setProjectionMatrix(glm::ortho(-400.0f, 400.0f, -400.0f, 400.0f, -1000.0f, 1000.0f));
	r.UpdateShaderData();

	stbi_set_flip_vertically_on_load(true);

	int x, y, c;
	uint8_t* MapTextureData = (uint8_t*)LoadImageData("ukMap.png", 1, &c, &x, &y);
	Texture2D MapTexture = Texture2D(MapTextureData, x, y, GL_RGBA, GL_RGBA, GL_TEXTURE0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	MapTexture.genMipmap();
	//BindExternalTex2dLd0(MapTexture.id);

	FreeImageData(MapTextureData);

	MapTextureData = (uint8_t*)LoadImageData("point.png", 0, &c, &x, &y);
	Texture2D HeliTexture = Texture2D(MapTextureData, x, y, GL_RGBA, GL_RGBA, GL_TEXTURE0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	MapTexture.genMipmap();
	//BindExternalTex2dLd0(HeliTexture.id);

	FreeImageData(MapTextureData);

	r.newModel(RENDER_MODEL_SQUARE1, Square, simpleProgram, 6, GL_TRIANGLES, MapTexture, 50);
	//r.newModel(RENDER_MODEL_HELICOPTER, Square, iconProgram, 6, GL_TRIANGLES, HeliTexture, 100000);

	RenderableMapSettings MapSetting;
	MapSetting.MoveX = 0.0f;
	MapSetting.MoveY = 0.0f;
	MapSetting.ScaleX = ScreenWidth / 2.0f;
	MapSetting.ScaleY = ScreenHeigth / 2.0f;
	MapSetting.CameraMatrix = lookAt(vec3(MapSetting.MoveX, MapSetting.MoveY, 0.0f), vec3(MapSetting.MoveX, MapSetting.MoveY, 0.0f) + LOOK_DIRECTION, vec3(0.0f, 1.0f, 0.0f));
	MapSetting.ScaleMatrix = glm::ortho(-400.0f, 400.0f, -400.0f, 400.0f, -1000.0f, 1000.0f);
	MapSetting.NeedUpdate = 0;

	r.setCameraMatrix(MapSetting.CameraMatrix);
	r.UpdateShaderDataCamera();



	win.customEventDispatch = std::bind(CustomEventDispatcher, std::placeholders::_1, &MapSetting);

#define OM(A) r.GetObjectMatrix(A)


	uint32_t MapRenderObject = r.newObject(RENDER_MODEL_SQUARE1, scale(mat4(1.0f), vec3(400.0f, 400.0f, 0.0f)));
	//r.newObject(RENDER_MODEL_HELICOPTER, translate(mat4(1.0f), vec3(300.0f, 35.0f, 0.05f)) * BaseIconScaleMatrix);
	//r.newObject(RENDER_MODEL_HELICOPTER, translate(mat4(1.0f), vec3(-114.0f, 27.0f, 0.05f))* BaseIconScaleMatrix);
	//r.newObject(RENDER_MODEL_HELICOPTER, translate(mat4(1.0f), vec3(56.0f, 11.0f, 0.05f))* BaseIconScaleMatrix);
	//r.newObject(RENDER_MODEL_HELICOPTER, translate(mat4(1.0f), vec3(231.0f, -65.0f, 0.05f))* BaseIconScaleMatrix);

	BezierProg.programGetDebugInfo(debugInfo, DEBUG_INFO_SPACE);
	printf(debugInfo);

	BezierCurveParameters p[4];
	p[0] = BezierCurveParameters{ vec2(0.0f, 0.0f), vec2(120.0f, 150.0f), vec2(0.0f, 200.0f) };
	p[1] = BezierCurveParameters{ vec2(-100.0f, -100.0f), vec2(150.0f, 70.0f), vec2(-100.0f, -200.0f) };

	BezierRenderer Bezier = BezierRenderer(BezierProg, 32, 50.0f);
	Bezier.UpdateData(p, 2, 0);

	glm::mat4 mt = glm::mat4(1.0f);

	clock_t evLoopStart = 0;
	clock_t evCurrTime = 0;
	clock_t evLoopTimeTarget = 1000;

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LINE_SMOOTH);
	glLineWidth(5.0f);
	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);

	CPUPerformanceTimer LoopElapsedTime = CPUPerformanceTimer();
	PerformanceTimer RenderElapsedTime = PerformanceTimer();
	RenderElapsedTime.Reset();
	LoopElapsedTime.Reset();
	uint32_t lp = 0;

	int64_t SumRenderTime = 0;

	PerlinNoiseGenerator generator;
	float x1, y1;
	int w, h, idx;
	w = 100;
	h = 100;
	float s = 0.1f;

	int size = w * h;
	char* texArr = new char[size];
	for (y1 = 0; y1 < h; y1++) {
		for (x1 = 0; x1 < w; x1++) {
			idx = y1 * w + x1;
			texArr[idx] = (char)((generator.perlin2DConfigurable(x1 * s, y1 * s, 121143, 1.0f, 8, 2.0f, 0.5f) * 0.5f + 0.5f) * 255.0f);
		}
	}

	//Texture2D newText = { texArr, w, h, GL_RED, GL_RGBA };
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	//newText.genMipmap();
	////r.newModel(RENDER_MODEL_SQUARE1, Square, simpleProgram, 6, GL_TRIANGLES, newText, 50);
	//r.BindActiveModel(RENDER_MODEL_SQUARE1);
	//r.md->std_texture2d = newText;


	AManager amanager{ r, Square, simpleProgram };
	while (true)
	{
		evLoopStart = clock();
		LoopElapsedTime.Start();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		RenderElapsedTime.TimeStart();
		r.RenderSelectedModel(RENDER_MODEL_SQUARE1);
		amanager.onUpdate();
		Bezier.Render(2);
		RenderElapsedTime.TimeEnd();

		if (MapSetting.NeedUpdate == 1)
		{
			MapSetting.NeedUpdate = 0;
			r.setCameraMatrix(MapSetting.CameraMatrix);
			r.setProjectionMatrix(MapSetting.ScaleMatrix);
			r.UpdateShaderData();
		}
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