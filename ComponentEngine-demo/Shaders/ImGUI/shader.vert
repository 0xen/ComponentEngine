#version 450

layout (location = 0) in vec2 inPos;
layout (location = 1) in vec2 inUV;
layout (location = 2) in vec4 inColor;

layout(set = 0, binding = 0) uniform UniformBufferObjectStatic {
    vec2 ScreenDim;
};

layout (location = 0) out vec2 outUV;
layout (location = 1) out vec4 outColor;

void main() 
{
	outUV = inUV;
	outColor = inColor;
	gl_Position = vec4(inPos * vec2(2.0f / ScreenDim.x, 2.0f / ScreenDim.y) + vec2(-1.0,-1.0), 0.0, 1.0);
}