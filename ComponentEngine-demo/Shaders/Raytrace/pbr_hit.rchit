#version 460
#extension GL_NV_ray_tracing : require
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_GOOGLE_include_directive : require

#include "HitgroupHelpers.glsl"


vec3 GenerateNormal(vec3 modelNormal, vec3 position, vec3 tangent, vec3 cameraDir,mat3 modelMatrix, int normalTextureId,inout vec2 UV, bool parallax)
{
	const float parallaxDepth = 0.06f;

	// Calculate inverse tangent matrix
	vec3 biTangent = cross(modelNormal, tangent);
	mat3 invTangentMatrix = mat3(tangent, biTangent, modelNormal);

	// Parallax mapping. Comment out for plain normal mapping
	if (parallax)
	{
		// Get camera direction in model space
		mat3 invWorldMatrix = transpose(modelMatrix);
		vec3 cameraModelDir = normalize(invWorldMatrix*cameraDir);

		// Calculate direction to offset UVs (x and y of camera direction in tangent space)
		mat3 tangentMatrix = transpose(invTangentMatrix);
		vec2 textureOffsetDir = (tangentMatrix*cameraModelDir).xy;

		// Offset UVs in that direction to account for depth (using height map and some geometry)
		float texDepth = parallaxDepth ;//* (HeightMap.Sample(TrilinearWrap, UV).r - 0.5f);
		UV += texDepth * textureOffsetDir;
	}

	// Extract normal from map and shift to -1 to 1 range
	vec3 textureNormal = 2.0f * normalize(texture(textureSamplers[normalTextureId], UV).xyz) - 1.0f;
	textureNormal.y = -textureNormal.y;

	// Convert normal from tangent space to world space
	return normalize(modelMatrix * (invTangentMatrix * textureNormal) );
}

vec3 GenerateTangent(vec3 normal)
{
	vec3 tangent;

	vec3 c1 = cross(normal, vec3(0.0, 0.0, 1.0));
	vec3 c2 = cross(normal, vec3(0.0, 1.0, 0.0));

	if (length(c1)>length(c2))
	{
	    tangent = c1;
	}
	else
	{
	    tangent = c2;
	}

	return normalize(tangent);

}

void main()
{
	rayPayload.depthTest = inRayPayload.depthTest;
	
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
	
  	const vec3 barycentrics = vec3(1.0 - attribs.x - attribs.y, attribs.x, attribs.y);



    int materialIndex = materials_mapping.map[(MaxMaterialsPerModel * gl_InstanceID) + v1.matIndex];
  	WaveFrontMaterial mat = unpackMaterial(materialIndex);


	vec2 texCoord = v0.texCoord * barycentrics.x + v1.texCoord * barycentrics.y +
	                        v2.texCoord * barycentrics.z;

	// Calculate normal
	vec3 f_normal = normalize(v0.nrm * barycentrics.x + v1.nrm * barycentrics.y + v2.nrm * barycentrics.z);
	
	mat3 modelMatrix = mat3(models.m[o.position]);

	//vec3 normal = normalize(modelMatrix * (f_normal * texture_normal));

 
	          

	// World position
	vec3 origin = gl_WorldRayOriginNV + gl_WorldRayDirectionNV * gl_HitTNV;

	vec3 viewVector = normalize(gl_WorldRayOriginNV - origin);

	vec3 tangent = GenerateTangent(f_normal);

	//vec3 normal = f_normal * normalize(texture(textureSamplers[mat.normalTextureId], texCoord).xyz) * modelMatrix;

	vec3 normal = GenerateNormal(f_normal, origin, tangent, viewVector, 
		modelMatrix, mat.normalTextureId, texCoord, true);

	/*vec3 normal = GenerateNormal(f_normal, origin, tangent, viewVector, 
		modelMatrix, mat.normalTextureId, texCoord, false);*/



	// Calculate the reflection angle
	vec3 reflectVec = reflect(-viewVector, normal);
	const uint currentResursion = inRayPayload.recursion;


	if(inRayPayload.depthTest)
	{
		inRayPayload.depth += length(gl_WorldRayDirectionNV * gl_HitTNV);
		if(currentResursion>0)
		{	
			//rayPayload.depthTest = true;
			rayPayload.depth = 0;
			rayPayload.recursion = currentResursion - 1;
			traceNV(topLevelAS, gl_RayFlagsOpaqueNV | gl_RayFlagsCullBackFacingTrianglesNV, 0xff, 0, 0, 0, origin, 0.00001f, normal, 1000.0, 1);

			inRayPayload.depth += rayPayload.depth;


			rayPayload.depth = 0;
			rayPayload.recursion = currentResursion - 1;
			traceNV(topLevelAS, gl_RayFlagsOpaqueNV | gl_RayFlagsCullBackFacingTrianglesNV,0xff, 0, 0, 0, origin, 0.00001f, reflectVec, 1000.0, 1);

			inRayPayload.depth += rayPayload.depth;
		}
		inRayPayload.recursion = currentResursion;

		for(uint i = 0 ; i < 100; i ++)
		{
			// Lighting Calc
			Light light = unpackLight(i);

			if(light.alive==0)
			{
			    continue;
			}

		    vec3 l = light.position - origin;

		    if(dot(normal,normalize(l))<0)
		    	continue;

			inRayPayload.depth += length(l);
		}

		inRayPayload.colour.xyz = camera.maxRecursionDepthColor;
		return;
	}




  	// PBR lab sheet

	// Texture maps             
	vec3 albedo = texture(textureSamplers[mat.textureId], texCoord).xyz;
	albedo = pow(albedo, vec3(GAMMA,GAMMA,GAMMA));
	float roughness = texture(textureSamplers[mat.roughnessTextureId], texCoord).r;
	float metalness = texture(textureSamplers[mat.metalicTextureId], texCoord).r;
	float cavity = texture(textureSamplers[mat.cavityTextureId], texCoord).r;
	float ao = texture(textureSamplers[mat.aoTextureId], texCoord).r;


	// Should be replaced
	//
	vec3 AmbientColour = vec3(0.5f,0.5f,0.5f);
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




	    vec3 l = origin - light.position;

	    float distance =  length(l);


	    isShadowed = true;

	    traceNV(topLevelAS, gl_RayFlagsTerminateOnFirstHitNV|gl_RayFlagsOpaqueNV|gl_RayFlagsSkipClosestHitShaderNV, 
	            0xFF, 0, 0,
	            SHADOW_MISS_SHADER_INDEX, origin, 0.00001f, l, distance, 2);



	    //if (!isShadowed)
	    {

		    float rdist = 1 / length(l);


			// Normalizing light
			l *= rdist;

			// Calculate light intensity
	        float  li = light.intensity * rdist * rdist;
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
			vec3 FSCHLICK = cSpec + (1 - cSpec) * ((1 - (ldoth)) * (1 - (ldoth)) * (1 - (ldoth)) * (1 - (ldoth)) * (1 - (ldoth)));


			float ndothSquare = ndoth * ndoth;
			float alpha = roughness * roughness;
			float asquare = alpha * alpha;
			float innerBracket = ndothSquare * (asquare - 1) + 1;
			float normdistri = asquare / (PI *  (innerBracket * innerBracket));


			float gone = ndotv / (ndotv * (1 - roughness / 2) + roughness / 2);
			float gtwo = ndotl / (ndotl * (1 - roughness / 2) + roughness / 2);
			float gdone = gone * gtwo;


			vec3 notend = (FSCHLICK * gdone * normdistri) / (4 * (ndotl) * (ndotv));

			vec3 f = flambert + notend;



			colour += PI * f * ((li * cavity) * lc) * ndotl;



	    }


	}



	colour = pow(colour, vec3(1/GAMMA,1/GAMMA,1/GAMMA));

	inRayPayload.colour.rgb = colour;



	/*
	vec3 globalIll = camera.maxRecursionDepthColor;
	vec3 globalIll2 = camera.maxRecursionDepthColor;


	if(currentResursion>0)
	{
		rayPayload.recursion = currentResursion - 1;
		traceNV(topLevelAS, gl_RayFlagsOpaqueNV | gl_RayFlagsCullBackFacingTrianglesNV, 0xff, 0, 0, MISS_SHADER_INDEX, origin, 0.00001f, normal, 1000.0, 1);

		globalIll = rayPayload.colour.xyz;


		rayPayload.recursion = currentResursion - 1;
		traceNV(topLevelAS, gl_RayFlagsOpaqueNV | gl_RayFlagsCullBackFacingTrianglesNV,0xff, 0, 0, MISS_SHADER_INDEX, origin, 0.00001f, reflectVec, 1000.0, 1);

		globalIll2 = rayPayload.colour.xyz;


		globalIll2.x = pow(globalIll2.x, 2) * 2;
		globalIll2.y = pow(globalIll2.y, 2) * 2;
		globalIll2.z = pow(globalIll2.z, 2) * 2;
	}
	inRayPayload.recursion = currentResursion;




	// Texture maps             
	vec3 albedo = texture(textureSamplers[mat.textureId], texCoord).xyz;
	albedo = pow(albedo, vec3(GAMMA,GAMMA,GAMMA));
	float roughness = texture(textureSamplers[mat.roughnessTextureId], texCoord).r;
	float metalness = texture(textureSamplers[mat.metalicTextureId], texCoord).r;
	float cavity = texture(textureSamplers[mat.cavityTextureId], texCoord).r;
	float ao = texture(textureSamplers[mat.aoTextureId], texCoord).r;


	float nDotV = max(dot(normal,viewVector), 0.001f);
	// Calculate how reflective the surface from 4% (Min) to 100%
	// Mix the minimum reflectiveness (4%) with the current albedo based on the range of 0-1
	vec3 specularColour = mix(vec3(0.00f,0.00f,0.00f), albedo, metalness);










	// At a glancing angle, how much should we increase the reflection
	// Microfacet specular - fresnel term - Slide 21
	vec3 F = specularColour + (1 - specularColour) * pow(max(1.0f - nDotV, 0.0f), 5.0f);


	vec3 colour = ((albedo  * (globalIll * ((1 - F) * (1 - roughness)))
	) + (F * (1 - roughness) * globalIll2)
	) * ao;



	for(uint i = 0; i < 100; i ++)
	{
		// Lighting Calc
		Light light = unpackLight(i);

		if(light.alive==0)
		{
		    continue;
		}




		uint sbtRecordOffset = 0;
		uint sbtRecordStride = 0;


	    vec3 l = light.position - origin;


	    if(dot(normal,normalize(l))<0)
	    	continue;

	    float rdist = 1 / length(l);


	    isShadowed = true;

	    traceNV(topLevelAS, gl_RayFlagsTerminateOnFirstHitNV|gl_RayFlagsOpaqueNV|gl_RayFlagsSkipClosestHitShaderNV, 
	            0xFF, 0, 0,
	            SHADOW_MISS_SHADER_INDEX, origin, 0.00001f, l, length(l), 2);


	    if (!isShadowed)
	    {
	        float  li = light.intensity * rdist * rdist;
	        vec3 lc = light.color;
	    
	    
	        // Vector that is half way between the view and the light
	        vec3 h = normalize(l + viewVector);
	    
	        float nDotL = max(dot(normal,normalize(l)), 0.001f);
	        float nDotH = max(dot(normal,h), 0.001f);
	    
	    
	        // Lambert diffuse - Slide 13
	        vec3 lambert = albedo / PI; // PI used for conservation of energy
	    
	    
	        // Reflected light - Microfacet
	        // Microfacet specular - normal distribution term - Slide 15-17
	        float alpha = max(roughness * roughness, 2.0e-3f); // Dividing by alpha in the dn term so don't allow it to reach 0
	        float alpha2 = alpha * alpha;
	        float nDotH2 = nDotH * nDotH;
	        float dn = nDotH2 * (alpha2 - 1) + 1;
	        float D = alpha2 / (PI * dn * dn);
	    
	    
	        // Microfacet specular - geometry term - Slide 23
	        float k = (roughness + 1);
	        k = k * k / 8;
	        float gV = nDotV / (nDotV * (1 - k) + k);
	        float gL = nDotL / (nDotL * (1 - k) + k);
	        float G = gV * gL;
	    
	        // BRDF - Slide 14 & 24 - Lambert is the diffuse term
	        vec3 brdf = lambert + F * G * D / (4 * nDotL * nDotV);
	    
	        colour += PI * li * lc * cavity * brdf * nDotL;
	    
	    }

	}


	colour = pow(colour, vec3(1/GAMMA,1/GAMMA,1/GAMMA));




	inRayPayload.colour.rgb = colour;*/
}


