#version 460
#extension GL_NV_ray_tracing : require
#extension GL_GOOGLE_include_directive : require

#include "Structures.glsl"

layout(location = 0) rayPayloadInNV RayPayload rayPayload;

void main()
{
	const float brightness = 0.3f;

	rayPayload.colour.rgb = vec3(1.0f,1.0f,1.0f);
	rayPayload.colour.rgb *= brightness;
}