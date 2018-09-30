#version 450

layout (set = 1, binding = 0) uniform sampler2D object_texture;

layout(location = 0) out vec4 outColor;

layout(location = 0) in vec2 inUV;
layout(location = 1) in vec3 inColor;

void main() 
{
	vec4 color = texture(object_texture, inUV, 0.0f);
    outColor = color;
    if(outColor.r<=0.01f)
    {
    	outColor = vec4(1.0f,1.0f,1.0f,1.0f);
    }
}