#version 460 core

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
layout (binding = 1) uniform atomic_uint cnt;
layout (binding = 2) uniform atomic_uint cnt_feedback;

void main()
{
	uint v = atomicCounterSubtract(cnt, atomicCounter(cnt)-1);
	atomicCounterExchange(cnt_feedback, v);
}