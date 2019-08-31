#version 460
#extension GL_NV_ray_tracing : require
#extension GL_EXT_nonuniform_qualifier : enable

struct RayPayload {
  vec3 color;
  float distance;
  vec3 normal;
  float reflector;
};

layout(location = 0) rayPayloadInNV RayPayload rayPayload;


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


struct Light
{
  vec3 position;
  float intensity;
  vec3 color;
  float alive;
};

Light unpackLight(uint index)
{
  uint startingIndex = index * 2;

  vec4 d0 = lights.l[index];
  vec4 d1 = lights.l[index + 1];

  Light l;
  l.position = vec3(d0.x,d0.y,d0.z);
  l.intensity = d0.w;
  l.color = vec3(d1.x,d1.y,d1.z);
  l.alive = d1.w;

  return l;
}


struct Offsets
{
  uint index;
  uint vertex;
  uint position;
};

Offsets unpackOffsets(uint index)
{
  uint startingIndex = index * 3;
  Offsets o;
  o.index = offsets.o[startingIndex];
  o.vertex = offsets.o[startingIndex + 1];
  o.position = offsets.o[startingIndex + 2];
  return o;
}

struct Vertex
{
  vec3 pos;
  vec3 nrm;
  vec3 color;
  vec2 texCoord;
  int matIndex;
};
// Number of vec4 values used to represent a vertex
uint vertexSize = 3;

Vertex unpackVertex(uint index, uint vertexOffset)
{
  Vertex v;

  vec4 d0 = vertices.v[vertexSize * (index + vertexOffset)];
  vec4 d1 = vertices.v[vertexSize * (index + vertexOffset) + 1];
  vec4 d2 = vertices.v[vertexSize * (index + vertexOffset) + 2];

  v.pos = d0.xyz;
  v.nrm = vec3(d0.w, d1.x, d1.y);
  v.color = vec3(d1.z, d1.w, d2.x);
  v.texCoord = vec2(d2.y, d2.z);
  v.matIndex = floatBitsToInt(d2.w);
  return v;
}

struct WaveFrontMaterial
{
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
  vec3 transmittance;
  vec3 emission;
  float shininess;
  float ior;      // index of refraction
  float dissolve; // 1 == opaque; 0 == fully transparent
  int illum;      // illumination model (see http://www.fileformat.info/format/material/)
  int textureId;
  int metalicTextureId;
  int roughnessTextureId;
};
// Number of vec4 values used to represent a material
const int sizeofMat = 6;

WaveFrontMaterial unpackMaterial(int matIndex)
{
  WaveFrontMaterial m;
  vec4 d0 = materials.m[sizeofMat * matIndex + 0];
  vec4 d1 = materials.m[sizeofMat * matIndex + 1];
  vec4 d2 = materials.m[sizeofMat * matIndex + 2];
  vec4 d3 = materials.m[sizeofMat * matIndex + 3];
  vec4 d4 = materials.m[sizeofMat * matIndex + 4];
  vec4 d5 = materials.m[sizeofMat * matIndex + 5];

  m.ambient = vec3(d0.x, d0.y, d0.z);
  m.diffuse = vec3(d0.w, d1.x, d1.y);
  m.specular = vec3(d1.z, d1.w, d2.x);
  m.transmittance = vec3(d2.y, d2.z, d2.w);
  m.emission = vec3(d3.x, d3.y, d3.z);
  m.shininess = d3.w;
  m.ior = d4.x;
  m.dissolve = d4.y;
  m.illum = int(d4.z);
  m.textureId = floatBitsToInt(d4.w);

  m.metalicTextureId = floatBitsToInt(d5.r);
  m.roughnessTextureId = floatBitsToInt(d5.g);

  return m;
}

void main()
{
  float PI = 3.14159265359f;


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

  rayPayload.normal = normal;

  vec2 texCoord = v0.texCoord * barycentrics.x + v1.texCoord * barycentrics.y +
                            v2.texCoord * barycentrics.z; 
              
  // World position
  vec3 origin = gl_WorldRayOriginNV + gl_WorldRayDirectionNV * gl_HitTNV;
  
  vec3 viewVector = normalize(gl_WorldRayOriginNV - origin);


  // Texture maps             
  vec3 albedo = texture(textureSamplers[mat.textureId], texCoord).xyz;
  float roughness = texture(textureSamplers[mat.roughnessTextureId], texCoord).r;
  float metalness = texture(textureSamplers[mat.metalicTextureId], texCoord).r;
  float cavity = 1.0f; // Temp
  float ao = cavity; // Temp



  
  rayPayload.distance = gl_RayTmaxNV;


  float nDotV = max(dot(normal,viewVector), 0.001f);
  // Calculate how reflective the surface from 4% (Min) to 100%
  float reflectivness = mix(0.04f, 1.0f, metalness);

  // Mix the minimum reflectiveness (4%) with the current albedo based on the range of 0-1
  vec3 specularColour = mix(vec3(0.0f, 0.0f, 0.0f), albedo, reflectivness);

  rayPayload.reflector *= metalness;

  // Calculate the reflection angle
  vec3 reflectVec = reflect(-viewVector, normal);




  // To do, global illumination /////////


    float tmin = 0.001;
  float tmax = 100.0;

    uint sbtRecordOffset = -1;
    uint sbtRecordStride = 0;
    uint missIndex = 0;
    isShadowed = true;

  RayPayload tempCurrentPayload = rayPayload;

    /*traceNV(topLevelAS, gl_RayFlagsTerminateOnFirstHitNV|gl_RayFlagsOpaqueNV|gl_RayFlagsSkipClosestHitShaderNV, 
            0xFF, sbtRecordOffset, sbtRecordStride,
            missIndex, origin, tmin, normal, tmax, 0);*/
  vec3 globalIll = vec3(0.2f,0.2f,0.2f);//rayPayload.color;

    /*traceNV(topLevelAS, gl_RayFlagsTerminateOnFirstHitNV|gl_RayFlagsOpaqueNV|gl_RayFlagsSkipClosestHitShaderNV, 
            0xFF, sbtRecordOffset, sbtRecordStride,
            missIndex, origin, tmin, reflectVec, tmax, 0);*/
  vec3 globalIll2 = vec3(0.2f,0.2f,0.2f);//rayPayload.color;
  
 rayPayload = tempCurrentPayload;

  //////////////////////////////////////



  // At a glancing angle, how much should we increase the reflection
  // Microfacet specular - fresnel term - Slide 21
  vec3 F = specularColour + (1 - specularColour) * pow(max(1.0f - nDotV, 0.0f), 5.0f);

  
  // DIFFRENT - We use globalIll for both
  vec3 colour = ((albedo  * (globalIll*(1 - F * (1 - roughness)))) + (F * (1 - roughness) * globalIll2)) * ao;




   for(uint i = 0 ; i < 100; i ++)
  {
    // Lighting Calc
    Light light = unpackLight(i);
    if(light.alive<0.1f)
    {
        continue;
    }
    vec3 l = light.position - origin;
    
    float rdist = 1 / length(l);
    // Normalise the light vector
    l *= rdist;
    

    uint sbtRecordOffset = 0;
    uint sbtRecordStride = 0;
    isShadowed = true;

    float tmax = 0.001;

    traceNV(topLevelAS, gl_RayFlagsTerminateOnFirstHitNV|gl_RayFlagsOpaqueNV|gl_RayFlagsSkipClosestHitShaderNV, 
            0xFF, SHADOW_SHADER_INDEX, sbtRecordStride,
            MISS_SHADER_INDEX, origin, tmin, l, rdist + 1.0f, 2);
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
  }



















  



  

  
  rayPayload.color = colour;


}


