#version 460 core

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

layout(std140, binding = 4) uniform OPS
{
	uint activeModelCounter;
	uint ops[];
}

layout(std430, binding = 5) buffer OPV
{
	mat4 matModel[];
};

layout(std430, binding = 6) buffer IDX
{
	uint modelIdx[];
};

void main()
{
	
	uint SWP = modelIdx[]


}