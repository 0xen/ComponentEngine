#version 460
#extension GL_NV_ray_tracing : require
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_GOOGLE_include_directive : require

#include "HitgroupHelpers.glsl"


void main()
{
	float rayDepth = length(gl_WorldRayDirectionNV * gl_HitTNV);
	

	const float opacity = 0.5f;

	const float PI = 3.14159265359f;
	const float GAMMA = 2.2f;


	Offsets o = unpackOffsets(gl_InstanceID);


  	ivec3 ind = ivec3(
  	indices.i[3 * gl_PrimitiveID + o.index], 
  	indices.i[3 * gl_PrimitiveID + 1 + o.index],
  	indices.i[3 * gl_PrimitiveID + 2 + o.index]);
	
  	Vertex v0 = unpackVertex(ind.x,o.vertex);
  	Vertex v1 = unpackVertex(ind.y,o.vertex);
  	Vertex v2 = unpackVertex(ind.z,o.vertex);
	


    int materialIndex = materials_mapping.map[(MaxMaterialsPerModel * gl_InstanceID) + v1.matIndex];
  	WaveFrontMaterial mat = unpackMaterial(materialIndex);

	const vec3 barycentrics = vec3(1.0 - attribs.x - attribs.y, attribs.x, attribs.y);
	vec2 texCoord = v0.texCoord * barycentrics.x + v1.texCoord * barycentrics.y +
		                        v2.texCoord * barycentrics.z;


	// World position
	vec3 origin = gl_WorldRayOriginNV + gl_WorldRayDirectionNV * gl_HitTNV;


	vec4 albedoWithAlpha = texture(textureSamplers[mat.textureId], texCoord).rgba;
	if(inRayPayload.responce == 1) // Shadow Test
	{
		float distanceToLight = inRayPayload.depth - rayDepth;
	    inRayPayload.depth = distanceToLight;
	    inRayPayload.origin = origin;
	    inRayPayload.direction = gl_WorldRayDirectionNV;
		inRayPayload.responce = 2;

    	inRayPayload.colour = inRayPayload.colour * vec4(albedoWithAlpha.rgb * (1.0f - opacity),0.0f);

    	//inRayPayload.colour *= inRayPayload.colour * vec4(albedoWithAlpha.rgb,0.0f);
    	//inRayPayload.colour *= 1.0f - shadowColorTransfurance;
		return;
	}
	else if(inRayPayload.responce == 4) // Are we doing a depth test
	{
		inRayPayload.depth = rayDepth;
		return;
	}

	inRayPayload.depth += rayDepth;

	mat3 modelMatrix = mat3(models.m[o.position]);

 
	          


	vec3 viewVector = normalize(gl_WorldRayDirectionNV);


	vec3 vertexTangent = GenerateTangent(v0.pos, v1.pos, v2.pos,
		v0.texCoord, v1.texCoord, v2.texCoord);

	vec3 v0t = normalize(vertexTangent - (dot(v0.nrm, vertexTangent)* v0.nrm));
	vec3 v1t = normalize(vertexTangent - (dot(v1.nrm, vertexTangent)* v1.nrm));
	vec3 v2t = normalize(vertexTangent - (dot(v2.nrm, vertexTangent)* v2.nrm));

	vec3 fTangent = normalize(HitBarycentrics(v0t, v1t,v2t, barycentrics));
	


	//const vec3 barycentrics = vec3(1.0 - attribs.x - attribs.y, attribs.x, attribs.y);
	vec3 f_normal = normalize(v0.nrm * barycentrics.x + v1.nrm * barycentrics.y + v2.nrm * barycentrics.z);
	

	//Calculate inverse tangent matrix
	vec3 biTangent = cross(f_normal, fTangent);
	mat3 invTangentMatrix = mat3(fTangent, biTangent, f_normal);

	vec3 normal = GenerateNormal(f_normal, fTangent, viewVector, 
		modelMatrix,invTangentMatrix, mat.normalTextureId, mat.heightTextureId, texCoord, true);


	// Calculate the reflection angle
	vec3 reflectVec = reflect(-viewVector, normal);

  	// PBR lab sheet

	// Texture maps      
	vec3 albedo = albedoWithAlpha.xyz;
	//vec3 albedo = texture(textureSamplers[mat.textureId], texCoord).xyz;
	albedo *= mat.diffuse;
	albedo = pow(albedo, vec3(GAMMA,GAMMA,GAMMA));
	float roughness = texture(textureSamplers[mat.roughnessTextureId], texCoord).r;
	float metalness = texture(textureSamplers[mat.metalicTextureId], texCoord).r;
	float cavity = texture(textureSamplers[mat.cavityTextureId], texCoord).r;
	float ao = texture(textureSamplers[mat.aoTextureId], texCoord).r;

	// Should be replaced
	//
	vec3 AmbientColour = vec3(0.3f,0.3f,0.3f);

	vec3 colour = (albedo * AmbientColour) * ao;

	ProcessLights(colour,albedo,roughness,metalness,cavity,origin,normal,viewVector);

	colour = pow(colour, vec3(1/GAMMA,1/GAMMA,1/GAMMA));


	inRayPayload.colour.rgb = colour;
	// Respond saying we are somewhat transparent
	inRayPayload.responce = 3;
	inRayPayload.origin = origin;
	inRayPayload.direction = gl_WorldRayDirectionNV;

	// Report how much light the object gives out
	inRayPayload.power = opacity;
}


