#version 460
#extension GL_NV_ray_tracing : require
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_GOOGLE_include_directive : require

#include "HitgroupHelpers.glsl"

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

  
  	WaveFrontMaterial mat = unpackMaterial(v1.matIndex);


	// Calculate normal
	vec3 f_normal = normalize(v0.nrm * barycentrics.x + v1.nrm * barycentrics.y + v2.nrm * barycentrics.z);
	mat3 normalMatrix = mat3(models.m[o.position]);
	normalMatrix = transpose(normalMatrix);
	vec3 normal = normalize(f_normal * normalMatrix);

	//inRayPayload.normal = normal;

	vec2 texCoord = v0.texCoord * barycentrics.x + v1.texCoord * barycentrics.y +
	                        v2.texCoord * barycentrics.z; 
	          
	normal = normalize(normal * texture(textureSamplers[mat.normalTextureId], texCoord).xyz);

	// World position
	vec3 origin = gl_WorldRayOriginNV + gl_WorldRayDirectionNV * gl_HitTNV;

	vec3 viewVector = normalize(gl_WorldRayOriginNV - origin);



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
	float cavity = 1.0f; // Temp
	float ao = 1.0f; // Temp


	float nDotV = max(dot(normal,viewVector), 0.001f);
	// Calculate how reflective the surface from 4% (Min) to 100%
	// Mix the minimum reflectiveness (4%) with the current albedo based on the range of 0-1
	vec3 specularColour = mix(vec3(0.04f,0.04f,0.04f), albedo, metalness);










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


		/*if(light.type == 0)
		{*/
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






		/*}
		else if(light.type == 1)
		{
		    vec3 l = light.dir;


		  
		    float rdist = 1 / length(l);
		    // Normalise the light vector
		    l *= rdist;

		    isShadowed = false;

		    traceNV(topLevelAS, gl_RayFlagsTerminateOnFirstHitNV|gl_RayFlagsOpaqueNV|gl_RayFlagsSkipClosestHitShaderNV, 
		            0xFF, SHADOW_SHADER_INDEX, sbtRecordStride,
		            MISS_SHADER_INDEX, origin, 0.001, l, 1000.0f, 2);



		    if (!isShadowed)
		    {
		        float  li = light.intensity * rdist * rdist;
		        vec3 lc = light.color;
		    
		    
		        // Vector that is half way between the view and the light
		        vec3 h = normalize(l + viewVector);
		    
		        float nDotL = max(dot(normal,l), 0.001f);
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
		}*/


	}


	colour = pow(colour, vec3(1/GAMMA,1/GAMMA,1/GAMMA));




	inRayPayload.colour.rgb = colour;
}


