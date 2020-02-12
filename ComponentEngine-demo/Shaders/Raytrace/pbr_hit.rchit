#version 460
#extension GL_NV_ray_tracing : require
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_GOOGLE_include_directive : require

#include "HitgroupHelpers.glsl"
/*vec3 GenerateNormal(vec3 modelNormal, vec3 tangent, int normalTextureId,vec2 UV)
{


	mat4x3 object_to_world = gl_ObjectToWorldNV;
	vec3 N = normalize(object_to_world * vec4(modelNormal, 0)).xyz;
	vec3 T = normalize(object_to_world * vec4(tangent, 0)).xyz;



	T = normalize(T - dot(T, N) * N);
	vec3 B = cross(N, T);

	const mat3 TBN = mat3(T, B, N);

	vec3 normal_t = normalize(texture(textureSamplers[normalTextureId], UV).xyz * 2.0f - 1.0f);
	vec3 fN = normalize(TBN * normal_t);

	//world_normal = N;

	return N;
}*/

vec3 GenerateNormal(vec3 modelNormal, vec3 tangent, vec3 cameraDir,
	mat3 modelMatrix, int normalTextureId, int heightTextureID,inout vec2 UV, bool parallax)
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
		vec3 cameraModelDir = normalize(cameraDir * invWorldMatrix);

		// Calculate direction to offset UVs (x and y of camera direction in tangent space)
		mat3 tangentMatrix = transpose(invTangentMatrix);
		vec2 textureOffsetDir = (cameraModelDir * tangentMatrix).xy;

		// Offset UVs in that direction to account for depth (using height map and some geometry)
		float texDepth = parallaxDepth * (texture(textureSamplers[heightTextureID], UV).r - 0.5f);
		UV += texDepth * textureOffsetDir;
	}

	// Extract normal from map and shift to -1 to 1 range
	vec3 textureNormal = 2.0f * normalize(texture(textureSamplers[normalTextureId], UV).xyz) - 1.0f;
	textureNormal.y = -textureNormal.y;

	// Convert normal from tangent space to world space
	return normalize((textureNormal * invTangentMatrix) * modelMatrix);
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

vec3 GenerateTangent(vec3 v1,vec3 v2,vec3 v3,vec2 u1,vec2 u2,vec2 u3)
{

	vec3 edge1 = v2 - v1;
	vec3 edge2 = v3 - v1;

	float s1 = u2.x - u1.x;
	float s2 = u3.x - u1.x;
	float t1 = u2.y - u1.y;
	float t2 = u3.y - u1.y;



	vec3 tangent;
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
	


    int materialIndex = materials_mapping.map[(MaxMaterialsPerModel * gl_InstanceID) + v1.matIndex];
  	WaveFrontMaterial mat = unpackMaterial(materialIndex);

	const vec3 barycentrics = vec3(1.0 - attribs.x - attribs.y, attribs.x, attribs.y);
	vec2 texCoord = v0.texCoord * barycentrics.x + v1.texCoord * barycentrics.y +
		                        v2.texCoord * barycentrics.z;
	//vec2 texCoord = vec2(normalize(HitAttribute(vec3(v0.texCoord,0.0f),vec3(v1.texCoord,0.0f),vec3(v2.texCoord,0.0f), attribs))).xy;


	mat4 modelMatrix = models.m[o.position];

	//vec3 normal = normalize(modelMatrix * (f_normal * texture_normal));

 
	          

	// World position
	vec3 origin = gl_WorldRayOriginNV + gl_WorldRayDirectionNV * gl_HitTNV;

	vec3 viewVector = normalize(gl_WorldRayDirectionNV);


	vec3 vertexTangent = GenerateTangent(v0.pos, v1.pos, v2.pos,
		v0.texCoord, v1.texCoord, v2.texCoord);

	vec3 v0t = normalize(vertexTangent - (dot(v0.nrm, vertexTangent)* v0.nrm));
	vec3 v1t = normalize(vertexTangent - (dot(v1.nrm, vertexTangent)* v1.nrm));
	vec3 v2t = normalize(vertexTangent - (dot(v2.nrm, vertexTangent)* v2.nrm));

	vec3 fTangent = normalize(HitBarycentrics(v0t, v1t,v2t, barycentrics));
	
	//vec3 f_normal = normalize(HitBarycentrics(v0.nrm, v1.nrm, v2.nrm, barycentrics));


	//vec3 tangent = GenerateTangent(f_normal);

	//modelMatrix = inverse(modelMatrix);
	//modelMatrix = transpose(modelMatrix);

	//vec3 normal = normalize(f_normal * mat3(modelMatrix));

	// Working ish
	/*mat3 test = mat3(
		vec3(gl_ObjectToWorldNV[0][0],gl_ObjectToWorldNV[1][0],gl_ObjectToWorldNV[2][0]),
		vec3(gl_ObjectToWorldNV[0][1],gl_ObjectToWorldNV[1][1],gl_ObjectToWorldNV[2][1]),
		vec3(gl_ObjectToWorldNV[0][2],gl_ObjectToWorldNV[1][2],gl_ObjectToWorldNV[2][2])
		);


	vec3 normal = normalize(test * f_normal).xyz;*/




	const vec3 barycentrics = vec3(1.0 - attribs.x - attribs.y, attribs.x, attribs.y);
	vec3 f_normal = normalize(v0.nrm * barycentrics.x + v1.nrm * barycentrics.y + v2.nrm * barycentrics.z);
	vec3 normal = normalize(gl_ObjectToWorldNV * vec4(f_normal,0)).xyz;


	//vec3 normal = GenerateNormal(f_normal, fTangent, mat.heightTextureId,texCoord);
	

	/*vec3 normal = GenerateNormal(f_normal, fTangent, viewVector, 
		test, mat.normalTextureId, mat.heightTextureId, texCoord, true);*/


	

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




	    vec3 l = light.position - origin;

	    float distance =  length(l);

	    //if(dot(normal,normalize(l))<0)
	    //	continue;

	    //isShadowed = true;

	    //traceNV(topLevelAS, gl_RayFlagsTerminateOnFirstHitNV|gl_RayFlagsOpaqueNV|gl_RayFlagsSkipClosestHitShaderNV, 
	    //        0xFF, 0, 0,
	    //        SHADOW_MISS_SHADER_INDEX, origin, 0.00001f, l, distance, 2);



	    //if (!isShadowed)
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

	inRayPayload.colour.rgb = colour;
	inRayPayload.colour.rgb = normal;
}


