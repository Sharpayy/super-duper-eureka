#version 430 core

layout(early_fragment_tests) in;

layout(std140, binding = 1) uniform MVP_DATA
{
	mat4 matProj;
	mat4 matCamera;
	mat4 matProjCamera;
};

uniform sampler2D image0;

uniform float MapScale;
uniform vec2 MapMove;

out vec4 FragColor;
in vec2 uv_o;

void main()
{

	//vec4 renderColor = vec4(1.0, 0.0, 0.2, 1.0);
	vec4 renderColor = texture(image0, MapMove + uv_o * MapScale);

	FragColor = renderColor;
}
