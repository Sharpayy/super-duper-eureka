#version 430 core

layout(early_fragment_tests) in;

layout(std140, binding = 1) uniform MVP_DATA
{
	mat4 matProj;
	mat4 matCamera;
	mat4 matProjCamera;
};

layout(binding = 4, r32f) uniform coherent image2D LDK_ENTRY;
layout(binding = 3) uniform atomic_uint LDKC;

out vec4 FragColor;

void main()
{
	vec4 renderColor = vec4(1.0, 0.0, 0.2, 1.0);

	FragColor = renderColor;
}
