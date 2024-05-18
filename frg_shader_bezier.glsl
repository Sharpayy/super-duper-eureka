#version 430

out vec4 OutColor;
uniform vec4 BezierColor;

void main()
{

	OutColor = vec4(BezierColor);
}