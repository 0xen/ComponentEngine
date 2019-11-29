#version 460
#extension GL_NV_ray_tracing : require
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_control_flow_attributes : require
#include "Structures.glsl"

layout(location = 0) rayPayloadInNV RayPayload inRayPayload;

layout(location = 1) rayPayloadNV RayPayload rayPayload;

layout(location = 2) rayPayloadNV bool isShadowed;

hitAttributeNV vec3 attribs;
layout(binding = 0, set = 0) uniform accelerationStructureNV topLevelAS;

layout(binding = 0, set = 1) buffer Vertices { vec4 v[]; }
vertices;
layout(binding = 1, set = 1) buffer Indices { uint i[]; }
indices;

layout(binding = 2, set = 1) buffer MatColorBufferObject { vec4[] m; }
materials;

layout(binding = 0, set = 2) uniform sampler2D[] textureSamplers;

layout(binding = 3, set = 1) buffer Lights { vec4 l[]; }
lights;

layout(binding = 0, set = 3) buffer ModelPos { mat4 m[]; }
models;

layout(binding = 1, set = 3) buffer ModelOffsets { uint o[]; }
offsets;



layout (constant_id = 0) const uint MISS_SHADER_INDEX = 0;
layout (constant_id = 1) const uint SHADOW_SHADER_INDEX = 0;


Light unpackLight(uint index)
{
  uint startingIndex = index * 4;

  vec4 d0 = lights.l[startingIndex];
  vec4 d1 = lights.l[startingIndex + 1];
  vec4 d2 = lights.l[startingIndex + 2];
  vec4 d3 = lights.l[startingIndex + 3];

  Light l;
  l.position = vec3(d0.x,d0.y,d0.z);
  l.intensity = d0.w;
  l.color = vec3(d1.x,d1.y,d1.z);
  l.alive = floatBitsToInt(d1.w);
  l.type = floatBitsToInt(d2.x);
  l.dir = vec3(d2.y,d2.z,d2.w);
  l.modelID = floatBitsToInt(d3.x);

  return l;
}

void main()
{

	inRayPayload.depth += length(gl_WorldRayDirectionNV * gl_HitTNV);
	


	[[unroll]]
	for(uint i = 0 ; i < 100; i ++)
	{
		Light light = unpackLight(i);

		if(light.alive==0)
		{
		    continue;
		}

		if(light.modelID >= 0 && light.modelID == gl_InstanceID)
		{
			inRayPayload.colour.xyz = light.color;
			return;
		}
	}


}


