#version 460
#extension GL_NV_ray_tracing : require
#extension GL_GOOGLE_include_directive : require

#include "Structures.glsl"

layout(location = 0) rayPayloadInNV RayPayload rayPayload;
layout(binding=5, set = 0) readonly uniform CameraBuffer {Camera camera; };

void main()
{
	//const float brightness = 0.3f;
	/*const float brightness = camera.globalIlluminationReflectionMissBrightness;


	rayPayload.colour.rgb = vec3(1.0f,1.0f,1.0f);
	rayPayload.colour.rgb *= brightness;*/


	const float brightness = camera.globalIlluminationReflectionMissBrightness;

	
   	// View-independent background gradient to simulate a basic sky background
	const vec3 gradientStart = vec3(0.0, 0.6, 1.0);
	const vec3 gradientEnd = vec3(1.0, 0.6, 0.0);
	vec3 unitDir = normalize(gl_WorldRayDirectionNV);
	float t = 0.5 * (unitDir.y + 1.0);
	rayPayload.colour.rgb = (1.0-t) * gradientEnd + t * gradientStart;
	rayPayload.colour.rgb *= brightness;
	rayPayload.depth += length(gl_WorldRayOriginNV) + gl_WorldRayDirectionNV.x + gl_WorldRayDirectionNV.y;
}