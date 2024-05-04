#version 450 core

layout (early_fragment_tests) in;

layout(std140, binding = 1) uniform MVP_DATA
{
	mat4 matProj;
	mat4 matCamera;
	mat4 matProjCamera;
};

layout(binding = 4, r32f) uniform coherent image2D LDK_ENTRY;

struct LDK_FRAG
{
	vec4 color;
	uint ptr;
	float depth;
};

layout(std430, binding = 2) buffer LK_OPC
{
	LDK_FRAG ldk[];
};

layout(binding = 3) uniform atomic_uint LDKC;
out vec4 FragColor;

in vec2 uv;
uniform sampler2D image0;

void main()
{
	vec4 renderColor = vec4(texture(image0, uv).xyz,1.0);

	FragColor = renderColor;
}

