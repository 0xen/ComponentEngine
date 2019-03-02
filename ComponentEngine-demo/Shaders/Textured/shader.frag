#version 450

layout (set = 1, binding = 0) uniform sampler2D diffuse_texture;

layout(location = 0) out vec4 outColor;


layout(location = 0) in vec2 inUV;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inDiffuseColor;
layout(location = 3) in vec3 inFragPos;
layout(location = 4) in vec3 inViewPos;



void main() 
{

	vec3 specular_material = vec3(1.0f);

	vec3 diffuse_material = texture(diffuse_texture, inUV, 0.0f).xyz * inDiffuseColor;

	vec4 final_color;

	final_color.xyz = diffuse_material;

	final_color.a = 1.0f; // No transparancy	

	outColor = final_color;



	
}