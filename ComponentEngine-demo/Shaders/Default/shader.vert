#version 450

layout(set = 0, binding = 0) uniform UniformBufferObjectStatic {
    mat4 view;
    mat4 proj;
}ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inUV;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec3 inColor;

layout(location = 4) in mat4 model;

layout(location = 0) out vec2 uv;
layout(location = 1) out vec3 color;


void main()
{
	mat4 MVP = ubo.proj * ubo.view * model;
	gl_Position = MVP * vec4(inPosition, 1.0);
	uv = inUV;
}