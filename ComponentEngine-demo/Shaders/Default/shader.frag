#version 450


layout(location = 0) out vec4 outColor;


layout(location = 0) in vec2 inUV;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inDiffuseColor;
layout(location = 3) in vec3 inFragPos;
layout(location = 4) in vec3 inViewPos;



void main() 
{
	// Temp static light

	vec3 light_position = vec3(0.0f, 100.0f, 100.0f);
	vec3 light_color = vec3(1.0f,1.0f,1.0f);



	float specularPower = 0.3f;


	// End of temp static light


	
    float ambientStrength = 0.1f;
    vec3 ambient_colour = ambientStrength * light_color;
	vec3 total_diffuse_light = ambient_colour;
	vec3 total_specular_light = vec3(0.0f, 0.0f, 0.0f);


{
	
	vec3 norm = normalize(inNormal);
	vec3 lightDir = normalize(light_position - inFragPos);  
	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = diff * light_color;
	total_diffuse_light+=diffuse;


    vec3 viewDir = normalize(inViewPos - inFragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularPower * spec * light_color;
	total_specular_light += specular;
}


	vec3 specular_material = vec3(1.0f);

	vec3 diffuse_material = inDiffuseColor;

	vec4 final_color;

	final_color.xyz = diffuse_material * total_diffuse_light + specular_material * total_specular_light;

	final_color.a = 1.0f; // No transparancy	

	outColor = final_color;



	
}