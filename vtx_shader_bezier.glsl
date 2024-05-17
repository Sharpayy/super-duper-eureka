#version 430 core

layout (location = 0) in vec2 pos;

layout(std140, binding = 1) uniform MVP_DATA
{
	mat4 matProj;
	mat4 matCamera;
	mat4 matProjCamera;
};

void main()
{
	gl_Position = vec4(pos, 0.0, 1.0);
}