#version 450 core

layout(local_size_x = 10, local_size_y = 10, local_size_z = 1) in;

layout(std140, binding = 1) uniform PERLIN_DATA
{
	float data[10];
} GEN_DATA;

layout(binding = 0, rgba32f) uniform image2D WRITE_TEXTURE;

void main()
{
	imageStore(WRITE_TEXTURE, ivec2(gl_GlobalInvocationID.xy), vec4(1.0));
}