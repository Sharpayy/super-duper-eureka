#version 430

layout(early_fragment_tests) in;

layout(std140, binding = 1) uniform MVP_DATA
{
	mat4 matProj;
	mat4 matCamera;
	mat4 matProjCamera;
};

uniform sampler2D image0;

out vec4 FragColor;
flat in float checko;
in vec2 uv_o;

void main()
{
	vec4 renderColor = texture(image0, uv_o);
		
	if (renderColor.a < 0.5)
		discard;

	if (checko > 0.0)
		renderColor = vec4(mix(renderColor.xyz, vec3(220.0, 20.0, 60.0) / 255.0, checko * 1.2 / 200.0), 1.0);
	
	FragColor = renderColor;
}
