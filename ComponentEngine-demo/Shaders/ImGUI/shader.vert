#version 450

layout (location = 0) in vec2 inPos;
layout (location = 1) in vec2 inUV;
layout (location = 2) in vec4 inColor;

layout (location = 0) out vec2 outUV;
layout (location = 1) out vec4 outColor;

void main() 
{
	outUV = inUV;
	outColor = inColor;
	gl_Position = vec4(inPos * vec2(2.0f / 1080, 2.0f / 720) + vec2(-1.0,-1.0), 0.0, 1.0);
}