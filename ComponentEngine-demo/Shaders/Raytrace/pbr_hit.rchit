#version 460
#extension GL_NV_ray_tracing : require
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_GOOGLE_include_directive : require

#include "HitgroupHelpers.glsl"

vec3 GenerateNormal(vec3 modelNormal, vec3 tangent, vec3 cameraDir,
	mat3 modelMatrix,mat3 invTangentMatrix, int normalTextureId, int heightTextureID,inout vec2 UV, bool parallax)
{
	const float parallaxDepth = 0.06f;

	// Parallax mapping. Comment out for plain normal mapping
	if (parallax)
	{
		// Get camera direction in model space
		mat3 invWorldMatrix = transpose(mat3(modelMatrix));
		vec3 cameraModelDir = normalize(invWorldMatrix * cameraDir);

		// Calculate direction to offset UVs (x and y of camera direction in tangent space)
		mat3 tangentMatrix = transpose(invTangentMatrix);
		vec2 textureOffsetDir = (tangentMatrix * cameraModelDir).xy;

		// Offset UVs in that direction to account for depth (using height map and some geometry)
		float texDepth = parallaxDepth * (texture(textureSamplers[heightTextureID], UV).r - 0.5f);
		UV += texDepth * textureOffsetDir;
	}

	// Extract normal from map and shift to -1 to 1 range
	vec3 textureNormal = normalize((2.0f * normalize(texture(textureSamplers[normalTextureId], UV).xyz)) - 1.0f);
	textureNormal.y = -textureNormal.y;

	// Convert normal from tangent space to world space
	//return normalize((textureNormal * invTangentMatrix) * modelMatrix).xyz;
	return normalize(modelMatrix * (invTangentMatrix * textureNormal)).xyz;
}

vec3 GenerateTangent(vec3 v1,vec3 v2,vec3 v3,vec2 u1,vec2 u2,vec2 u3)
{

	vec3 tangent;
	vec3 edge1 = v2 - v1;
	vec3 edge2 = v3 - v1;

	float s1 = u2.x - u1.x;
	float s2 = u3.x - u1.x;
	float t1 = u2.y - u1.y;
	float t2 = u3.y - u1.y;



	float denom = (s1 * t2) - (s2 * t1);
	if (!(abs(denom) < 0.00001))
	{
		tangent = ((t2 * edge1) - (t1 * edge2)) / ((s1 * t2) - (s2 * t1));
	}
	else
	{
		tangent = vec3(1.0f,0.0f,0.0f);
	}
	return tangent;
}

vec3 HitBarycentrics(vec3 a, vec3 b, vec3 c, vec3 bary)
{
	//return a + bary.x * (b - a) + bary.y * (c - a);
	return a * bary.x + b * bary.y + c * bary.z;
}

void main()
{
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
	if(inRayPayload.responce == 1)
	{
		if(albedoWithAlpha.a < 0.9f)
		{
			rayPayload.responce = 1; // We are doing a shadow test

			float distanceFromStart = length(gl_WorldRayOriginNV - origin);
			float distanceToLight = gl_RayTmaxNV - distanceFromStart;

		    traceNV(topLevelAS, gl_RayFlagsOpaqueNV | gl_RayFlagsCullBackFacingTrianglesNV, 0xFF, 0, 0, 
		    	SHADOW_MISS_SHADER_INDEX, origin,
		             0.00001f, gl_WorldRayDirectionNV, distanceToLight,1);

		    inRayPayload.responce = rayPayload.responce;
		}
		else
		{
			inRayPayload.responce = 1;
		}
		return;
	}
	if(albedoWithAlpha.a < 0.9f)
	{
		inRayPayload.responce = 2;
		inRayPayload.origin = origin;
		inRayPayload.direction = gl_WorldRayDirectionNV;
		return;
	}











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
	//

	vec3 colour = (albedo * AmbientColour) * ao;


	for(uint i = 0; i < 100; i ++)
	{
		// Lighting Calc
		Light light = unpackLight(i);

		if(light.alive==0)
		{
		    continue;
		}




	    vec3 l = light.position - origin;

	    float distance =  length(l);

	    //if(dot(normal,normalize(l))<0)
	    //	continue;

		

		rayPayload.responce = 1; // We are doing a shadow test

	    traceNV(topLevelAS, gl_RayFlagsOpaqueNV | gl_RayFlagsCullBackFacingTrianglesNV, 
	            0xFF, 0, 0, SHADOW_MISS_SHADER_INDEX, origin,
	             0.00001f + light.shadowRangeStartOffset,
	              normalize(l), distance + light.shadowRangeEndOffset, 1);



	    if (rayPayload.responce==0)
	    {

		    float rdist = 1.0f / length(l);


			// Normalizing light
			l *= rdist;

			// Calculate light intensity
	        float li = light.intensity * rdist * rdist;
			// Lights color
			vec3 lc = light.color;

		  	// Halfway vector (normal halfway between view and light vector)
	        vec3 h = normalize(l + viewVector);


			float ndotv = dot(normal, viewVector);
			if (ndotv < 0) ndotv = 0.001f;

			float ndotl = dot(normal, l);
			if (ndotl < 0) ndotl = 0.001f; 

			float ndoth = dot(normal, h);
			if (ndoth < 0) ndoth = 0.001f;






			vec3 flambert = albedo / PI;


			float ldoth = dot(l, h);

			float nonMetalSpecular = 0.04f;
			vec3 cSpec = mix(vec3(nonMetalSpecular,nonMetalSpecular,nonMetalSpecular), albedo, metalness); 
			vec3 FSCHLICK = cSpec + (1.0f - cSpec) * ((1.0f - (ldoth)) * 
				(1.0f - (ldoth)) * (1.0f - (ldoth)) * (1.0f - (ldoth)) * (1.0f - (ldoth)));


			float ndothSquare = ndoth * ndoth;
			float alpha = roughness * roughness;
			float asquare = alpha * alpha;
			float innerBracket = ndothSquare * (asquare - 1.0f) + 1.0f;
			float normdistri = asquare / (PI *  (innerBracket * innerBracket));


			float gone = ndotv / (ndotv * (1.0f - roughness / 2.0f) + roughness / 2.0f);
			float gtwo = ndotl / (ndotl * (1.0f - roughness / 2.0f) + roughness / 2.0f);
			float gdone = gone * gtwo;


			vec3 notend = (FSCHLICK * gdone * normdistri) / (4.0f * (ndotl) * (ndotv));

			vec3 f = flambert + notend;



			colour += PI * f * ((li * cavity) * lc) * ndotl;



	    }
	}



	colour = pow(colour, vec3(1/GAMMA,1/GAMMA,1/GAMMA));

	// Respond saying we are done
	inRayPayload.responce = 0;
	inRayPayload.colour.rgb = colour;
	//inRayPayload.colour.rgb = normal;*/
}


