#version 430

layout(early_fragment_tests) in;

layout(std140, binding = 1) uniform MVP_DATA
{
	mat4 matProj;
	mat4 matCamera;
	mat4 matProjCamera;
};

uniform sampler2D image0;
uniform uint SelectedModelId;

out vec4 FragColor;
flat in uint ModelId;
flat in float checko;
in vec2 uv_o;

void main()
{
	vec4 renderColor = texture(image0, uv_o);
	
	if (renderColor.a < 0.5)
		discard;

	if (checko == 1)
		renderColor = vec4(0.67, 0.85, 0.9, 1.0);

	FragColor = renderColor;
}
