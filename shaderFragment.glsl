#version 450 core
#extension EXT_shader_image_load_store: enable

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

in vec2 uv;
uniform sampler2D image0;

//out vec4 Out;

void main()
{
	vec4 renderColor = texture(image0, uv) * vec4(1.0, 1.0, 1.0, 0.5);

	//uint idx = 5;
	//uint prevIdx = 4;
	uint idx = atomicCounterIncrement(LDKC);
	uint prevIdx = uint(imageAtomicExchange(LDK_ENTRY, ivec2(gl_FragCoord), float(idx)));


	LDK_FRAG newFrag;
	newFrag.color = renderColor;
	newFrag.ptr = prevIdx;
	newFrag.depth = gl_FragCoord.z;

	ldk[idx] = newFrag;
}