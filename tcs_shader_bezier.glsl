#version 430
layout (vertices = 3) out;

uniform float BezierQuality;

void main()
{
		gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;

	    gl_TessLevelOuter[0] = BezierQuality;
        gl_TessLevelOuter[1] = BezierQuality;
        gl_TessLevelOuter[2] = BezierQuality;
        gl_TessLevelOuter[3] = BezierQuality;

        gl_TessLevelInner[0] = BezierQuality;
        gl_TessLevelInner[1] = BezierQuality;

}