#version 430 core

layout(location = 0) in vec2 pos;
layout(location = 1) in vec2 uv_c;
layout(location = 2) in float check;

layout(std140, binding = 1) uniform MVP_DATA
{
	mat4 matProj;
	mat4 matCamera;
	mat4 matProjCamera;
};

layout(std430, binding = 5) restrict buffer OPV 
{
	mat4 matModel[];
};

layout(std430, binding = 6) restrict buffer IDX
{
	uint modelIdx[];
};

out vec2 uv_o;
flat out float checko;
flat out uint ModelId;
uniform float uIconScale;

void main()
{
	uint MdlId = modelIdx[gl_InstanceID];
	mat4 mModel = matModel[MdlId];

	mModel[0].x = mModel[0].x * uIconScale;
	mModel[1].y = mModel[1].y * uIconScale;
	mModel[2].z = mModel[2].z * uIconScale;

	gl_Position = matProjCamera * mModel * vec4(pos, 0.0, 1.0);

	uv_o = uv_c;
	ModelId = MdlId;
	checko = check;
}