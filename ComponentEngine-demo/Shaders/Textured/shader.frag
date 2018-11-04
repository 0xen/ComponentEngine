#version 450

layout (set = 1, binding = 0) uniform sampler2D diffuse_texture;

layout(location = 0) out vec4 outColor;


layout(location = 0) in vec2 inUV;


struct LightInstanceData
{
	vec4 diffuse_color;
	vec4 world_pos;
	vec4 proj_pos;
	vec4 world_normal;
	vec4 camera_position;
};

layout(location = 1) in LightInstanceData in_light_data;





void main() 
{

	vec4 world_normal = normalize(in_light_data.world_normal);
	vec3 camera_dir = normalize(in_light_data.camera_position.xyz - in_light_data.world_pos.xyz);

	vec3 ambient_colour = vec3(0.2f,0.2f,0.2f);

	// Temp static light

	vec4 light_position = vec4(8.0f,0.0f,0.0f,0.0f);
	vec4 light_color = vec4(1.0f,1.0f,1.0f,1.0f);
	vec4 light_facing = normalize(vec4(1.0f,0.0f,0.0f,0.0f));
	float SpecularPower = 256.0f;

	vec3 light_direction = normalize(light_position.xyz * in_light_data.world_pos.xyz);

	// End of temp static light


	vec3 total_diffuse_light = ambient_colour;
	vec3 total_specular_light = vec3(0.0f, 0.0f, 0.0f);



	if(dot(light_facing.xyz,-light_direction) > cos(45.0f))
	{
		vec3 diffuse_light = light_color.xyz * max(dot(world_normal.xyz,light_direction),0.0f);
		vec3 halfway = normalize(light_direction + camera_dir);
		vec3 specular_light = light_color.xyz * pow(max(dot(world_normal.xyz,halfway),0.0f),SpecularPower);
		total_diffuse_light += diffuse_light;
		total_specular_light += specular_light;
	}



	vec3 specular_material = vec3(1.0f);

	vec3 diffuse_material = texture(diffuse_texture, inUV, 0.0f).xyz * in_light_data.diffuse_color.xyz;

	vec4 final_color;

	final_color.xyz = diffuse_material * total_diffuse_light + specular_material * total_specular_light;

	final_color.a = 1.0f; // No transparancy	

	outColor = final_color;



	
}