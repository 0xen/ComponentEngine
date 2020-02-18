#version 460
#extension GL_NV_ray_tracing : require
#extension GL_GOOGLE_include_directive : require

#include "Structures.glsl"

layout(location = 1) rayPayloadInNV RayPayload inRayPayload;

void main()
{
	inRayPayload.responce = 0;
}