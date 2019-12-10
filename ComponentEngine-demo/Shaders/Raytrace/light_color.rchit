#version 460
#extension GL_NV_ray_tracing : require
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_control_flow_attributes : require

#include "HitgroupHelpers.glsl"

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


