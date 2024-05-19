#pragma once
//Renderer
#include <cstdio>
#include "iWinSDL.h"
#include "iRendered.h"
#include "iReaders.h"
#include <ctime>
#include <gtc/matrix_transform.hpp>
#include <functional>
#include <stb_image.h>

//Additional
#include "perlin.h"

#include "Aircraft.h"
#include <utility>
#include <random>

#define BEZIER_PATCH_AMOUNT 4

using namespace glm;

typedef struct BezierCurveParameters
{
	vec2 str_pos;
	vec2 mid0_pos;
	vec2 mid1_pos;
	vec2 end_pos;
};

class BezierRenderer
{
public:
	Program bezierProgram;
	VertexBuffer bezierVertex;
	Buffer<GL_ARRAY_BUFFER> bezierData;
	uint32_t count;
	uint32_t maxCount;

	float quality;
	vec4 color;

	uint32_t ulQuality;
	uint32_t ulColor;

	BezierRenderer(Program program, uint32_t cnt, float quality_);
	void Render(uint32_t amount);
	void UpdateData(BezierCurveParameters* data, uint32_t amount, uint32_t amountOffset);
	void SetQuality(float quality_);
	void SetColor(vec4 color_);

};


