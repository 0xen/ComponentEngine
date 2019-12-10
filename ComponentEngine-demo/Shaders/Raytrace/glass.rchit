#version 460
#extension GL_NV_ray_tracing : require
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_control_flow_attributes : require

#include "HitgroupHelpers.glsl"


void main()
{
	rayPayload.depthTest = inRayPayload.depthTest;
	Offsets o = unpackOffsets(gl_InstanceID);


  	ivec3 ind = ivec3(
  	indices.i[3 * gl_PrimitiveID + o.index], 
  	indices.i[3 * gl_PrimitiveID + 1 + o.index],
  	indices.i[3 * gl_PrimitiveID + 2 + o.index]);
	
  	Vertex v0 = unpackVertex(ind.x,o.vertex);
  	Vertex v1 = unpackVertex(ind.y,o.vertex);
  	Vertex v2 = unpackVertex(ind.z,o.vertex);
	
  	const vec3 barycentrics = vec3(1.0 - attribs.x - attribs.y, attribs.x, attribs.y);

  	WaveFrontMaterial mat = unpackMaterial(v1.matIndex);

	// Calculate normal
	vec3 f_normal = normalize(v0.nrm * barycentrics.x + v1.nrm * barycentrics.y + v2.nrm * barycentrics.z);
	mat3 normalMatrix = mat3(models.m[o.position]);
	normalMatrix = transpose(normalMatrix);
	vec3 normal = normalize(f_normal * normalMatrix);

	vec2 texCoord = v0.texCoord * barycentrics.x + v1.texCoord * barycentrics.y +
		                        v2.texCoord * barycentrics.z; 
	 

	normal = normalize(normal * texture(textureSamplers[mat.normalTextureId], texCoord).xyz);






	// World position
	vec3 origin = gl_WorldRayOriginNV + gl_WorldRayDirectionNV * gl_HitTNV;

	vec3 viewVector = normalize(gl_WorldRayOriginNV - origin);

	const uint currentResursion = inRayPayload.recursion;
	
	uint cullFlag;
	vec3 refractVec;

	if(((gl_IncomingRayFlagsNV >> gl_RayFlagsCullBackFacingTrianglesNV) & 0x1) == 0x1)
	{
		cullFlag = gl_RayFlagsCullFrontFacingTrianglesNV ;
		refractVec = refract(viewVector, normal, 1.33f);
	}
	else
	{
		cullFlag = gl_RayFlagsCullBackFacingTrianglesNV;
		refractVec = refract(viewVector, normal, 1.0f);
	}


	if(inRayPayload.depthTest)
	{
		inRayPayload.depth += length(gl_WorldRayDirectionNV * gl_HitTNV);

		//rayPayload.depthTest = true;

		if(currentResursion>0)
		{
			//rayPayload.depthTest = true;


			rayPayload.depth = 0;
			rayPayload.recursion = currentResursion - 1;
			traceNV(topLevelAS, gl_RayFlagsOpaqueNV | cullFlag, 0xff, 0, 0, 0, origin, 0.00001f, refractVec, 1000.0, 1);

			inRayPayload.depth += rayPayload.depth;
		}


		inRayPayload.colour.xyz = camera.maxRecursionDepthColor;
		return;
	}

	vec3 color = camera.maxRecursionDepthColor;
	if(currentResursion>0)
	{
		rayPayload.recursion = currentResursion - 1;


		traceNV(topLevelAS, gl_RayFlagsOpaqueNV | cullFlag,0xff, 0, 0, 0, origin, 0.00001f, refractVec, 1000.0, 1);

		color = rayPayload.colour.xyz;
	}
	inRayPayload.recursion = currentResursion;
	inRayPayload.colour.xyz = color;
}


