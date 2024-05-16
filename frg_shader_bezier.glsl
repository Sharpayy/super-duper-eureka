#version 410 core

out vec4 OutColor;
uniform vec4 BezierColor;

void main()
{
	OutColor = vec4(BezierColor);
}