#version 450


layout(set = 0, binding = 0) uniform UniformBufferObjectStatic {
    mat4 view;
    mat4 proj;
}ubo;

layout(location = 0) in vec4 inPosition;
layout(location = 1) in vec4 inColor;


layout(location = 0) out vec4 outColor;
layout(location = 1) out float outScale;

void main()
{
	outColor = inColor;
	outScale = inPosition.w;
	
	vec4 model_pos = vec4(inPosition.xyz,1.0f);

	gl_Position = ubo.proj * ubo.view * model_pos;

}