#version 450

layout(set = 0, binding = 0) uniform UniformBufferObjectStatic {
    mat4 view;
    mat4 proj;
}ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inUV;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec3 inDiffuseColor;

layout(location = 4) in mat4 world_matrix;

layout(location = 0) out vec2 uv;


struct LightInstanceData
{
	vec4 diffuse_color;
	vec4 world_pos;
	vec4 proj_pos;
	vec4 world_normal;
	vec4 camera_position;
};

layout(location = 1) out LightInstanceData light_data;



void main()
{

	vec4 model_pos = vec4(inPosition,1.0f);

	vec4 world_pos = model_pos * world_matrix;
	light_data.world_pos = world_pos;

	vec4 view_pos = world_pos * ubo.view;
	vec4 proj_pos = view_pos * ubo.proj;
	light_data.proj_pos = proj_pos;

	vec4 camera_position = ubo.view[3];

	vec4 model_normal = vec4(inNormal,0.0f);
	vec4 world_normal = model_normal * world_matrix;
	light_data.world_normal = normalize(world_normal);


	mat4 MVP = ubo.proj * ubo.view * world_matrix;
	gl_Position = MVP * model_pos;
	uv = inUV;


	// Light data
	light_data.diffuse_color = vec4(inDiffuseColor,1.0f);
	light_data.camera_position = camera_position;
}