#version 430 
layout (isolines, fractional_odd_spacing, ccw) in;

layout(std140, binding = 1) uniform MVP_DATA
{
	mat4 matProj;
	mat4 matCamera;
	mat4 matProjCamera;
};

void main()
{
	float t = gl_TessCoord.x;

	vec2 p0 = gl_in[0].gl_Position.xy;
	vec2 p1 = gl_in[1].gl_Position.xy;
	vec2 p2 = gl_in[2].gl_Position.xy;

	vec2 Point = (1-t)*(1-t)*p0 + 2*(1-t)*t*p1 + t*t*p2;

	gl_Position = matProjCamera * vec4(Point, 0.5, 1.0);
}