#version 450

layout(set = 0, binding = 0) uniform UniformBufferObjectStatic {
    mat4 view;
    mat4 proj;
}ubo;

layout(location = 0) in vec4 inPosition;

layout(location = 1) in mat4 model;


void main()
{
	mat4 MVP = ubo.proj * ubo.view * model;
	gl_Position = MVP * inPosition;
}