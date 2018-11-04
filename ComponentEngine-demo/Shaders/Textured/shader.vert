#version 450

layout(set = 0, binding = 0) uniform UniformBufferObjectStatic {
    mat4 view;
    mat4 proj;
}ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inUV;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec3 inDiffuseColor;

layout(location = 4) in mat4 model_matrix;

layout(location = 0) out vec2 uv;
layout(location = 1) out vec3 normal;
layout(location = 2) out vec3 diffuseColor;
layout(location = 3) out vec3 fragPos;
layout(location = 4) out vec3 viewPos;



void main()
{
	
	vec4 model_pos = vec4(inPosition,1.0f);
	vec4 world_position = model_matrix * model_pos;
	gl_Position = ubo.proj * ubo.view * world_position;
	uv = inUV;
	diffuseColor = inDiffuseColor;

	fragPos = world_position.xyz;
    normal = (model_matrix * vec4(inNormal, 0.0f)).xyz;


    viewPos = ubo.view[3].xyz;


}