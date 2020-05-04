#include "Structures.glsl"
#include "Maths.glsl"

//////////////////////////////////
// CROSS SHADER VARIABLE SHARING//
//////////////////////////////////



////////////////////
// Shader inputs //
////////////////////

layout(location = 0) rayPayloadInNV RayPayload inRayPayload;

////////////////////
// Shader outputs //
////////////////////

layout(location = 1) rayPayloadNV RayPayload rayPayload;


///////////////////////////
// Descriptor Set Inputs //
///////////////////////////

hitAttributeNV vec3 attribs;
layout(binding = 0, set = 0) uniform accelerationStructureNV topLevelAS;

layout(binding=5, set = 0) readonly uniform CameraBuffer {Camera camera; };

layout(binding = 0, set = 1) buffer Vertices { vec4 v[]; }
vertices;
layout(binding = 1, set = 1) buffer Indices { uint i[]; }
indices;

layout(binding = 2, set = 1) buffer MatColorBufferObject { vec4[] m; }
materials;

layout(binding = 3, set = 1) buffer Lights { vec4 l[]; }
lights;

uint MaxMaterialsPerModel = 512;

layout(binding = 4, set = 1) buffer MaterialsMapping { int map[]; }
materials_mapping;

layout(binding = 0, set = 2) uniform sampler2D[] textureSamplers;

layout(binding = 0, set = 3) buffer ModelPos { mat4 m[]; }
models;

layout(binding = 1, set = 3) buffer ModelPosIT { mat4 m[]; }
modelsIT;

layout(binding = 2, set = 3) buffer ModelOffsets { uint o[]; }
offsets;



layout (constant_id = 0) const uint SHADOW_MISS_SHADER_INDEX = 0;
layout (constant_id = 1) const uint MISS_SHADER_INDEX = 0;


const float PI = 3.14159265359f;
const float GAMMA = 2.2f;


  /////////////////////////////////
 // Structure Unpacking Helpers //
/////////////////////////////////


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
  l.shadowRangeStartOffset = d3.y;
  l.shadowRangeEndOffset = d3.z;

  return l;
}


Offsets unpackOffsets(uint index)
{
  uint startingIndex = index * 3;
  Offsets o;
  o.index = offsets.o[startingIndex];
  o.vertex = offsets.o[startingIndex + 1];
  o.position = offsets.o[startingIndex + 2];
  return o;
}

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

// Number of vec4 values used to represent a material
const int sizeofMat = 7;

WaveFrontMaterial unpackMaterial(int matIndex)
{
  WaveFrontMaterial m;
  vec4 d0 = materials.m[sizeofMat * matIndex + 0];
  vec4 d1 = materials.m[sizeofMat * matIndex + 1];
  vec4 d2 = materials.m[sizeofMat * matIndex + 2];
  vec4 d3 = materials.m[sizeofMat * matIndex + 3];
  vec4 d4 = materials.m[sizeofMat * matIndex + 4];
  vec4 d5 = materials.m[sizeofMat * matIndex + 5];
  vec4 d6 = materials.m[sizeofMat * matIndex + 6];

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
  m.normalTextureId = floatBitsToInt(d5.b);
  m.cavityTextureId = floatBitsToInt(d5.a);
  m.aoTextureId = floatBitsToInt(d6.r);
  m.heightTextureId = floatBitsToInt(d6.g);

  return m;
}


  //////////////////////////
 // Helper RTX Functions //
//////////////////////////

vec3 PointOnSphere(inout uint seed, vec3 center, float rad)
{
    vec3 res = vec3(
    RandomFloat(seed,-1.0f,1.0f),
    RandomFloat(seed,-1.0f,1.0f),
    RandomFloat(seed,-1.0f,1.0f)
    );

    res = normalize(res);

    res *= rad;

    return center - res;
}

uint SpawnShadowRay(vec3 origin, vec3 direction, float length)
{
  rayPayload.responce = 1; // We are doing a shadow test
  rayPayload.depth = length;
  traceNV(topLevelAS, gl_RayFlagsOpaqueNV, 
          0xFF, 0, 0, SHADOW_MISS_SHADER_INDEX, origin, 0.00001f, direction, length, 1);


  return rayPayload.responce;
}

uint SpawnShadowRay(vec3 origin, vec3 direction, float length, uint flags)
{
  rayPayload.responce = 1; // We are doing a shadow test
  rayPayload.depth = length;
  traceNV(topLevelAS, gl_RayFlagsOpaqueNV | flags, 
          0xFF, 0, 0, SHADOW_MISS_SHADER_INDEX, origin, 0.001f, direction, length, 1);


  return rayPayload.responce;
}


void Shadow(inout uint responce, uint maxRecursion, vec3 origin, vec3 shadowRayDirection)
{
    float shadowRayDistance = length(shadowRayDirection);
    shadowRayDirection = normalize(shadowRayDirection);

    if(inRayPayload.depth < 20.0f) // Should we use accurate shadows?
    {
      uint flags = gl_RayFlagsCullBackFacingTrianglesNV;

      if(inRayPayload.depth < 10.0f)
      {
          flags = 0;
      }

      while(true)
      {
        responce = SpawnShadowRay(origin,shadowRayDirection,shadowRayDistance,flags);
        if (responce == 2) // Transparent
        {
            maxRecursion--;
            origin = rayPayload.origin;
            shadowRayDirection = rayPayload.direction;
            shadowRayDistance = rayPayload.depth;
        }
        else
        {
          // We have hit the light or hit a solid object
          break;
        }
        // We have went through too many transparent objects, give up
        if(maxRecursion<=0)break;
      }
    }
    else // Use single pass shadows
    {
        responce = SpawnShadowRay(origin,shadowRayDirection,shadowRayDistance);
        // If it hit a transparent object, override and change to be a shadow
        if(responce==2)responce=1;
    }
}



void ProcessLights(inout vec3 colour,vec3 albedo,float roughness,float metalness,float cavity, 
                    vec3 origin, vec3 normal, vec3 viewVector)
{
  rayPayload.seed = inRayPayload.seed;
  for(uint i = 0; i < 100; i ++)
  {
    // Lighting Calc
    Light light = unpackLight(i);

    if(light.alive==0)
    {
        continue;
    }


    vec3 l;

    float distance;

    if(light.type==0)
    {
      l = light.position - origin;
      distance =  length(l);
    }
    else
    {
      l = light.dir;
      distance = 1000.0f;
    }

    /*if(dot(normal,normalize(l))<0)
      continue;*/


    uint responce = 0;

    rayPayload.colour = vec4(light.color,0.0f);
    
    uint maxRecursion = camera.recursionCount;

    vec3 shadowRayOrigin = origin;

    rayPayload.recursion = camera.gpuRecursionCount;

    float shadowPower = 0.0f;
    int litSamples = 0;

    vec3 processedLightColor = vec3(0,0,0);

    bool softShadows = true;


    if(softShadows)
    {

      int sampleCount = 50;

      vec3 shadowRayDirection = l;
      for(int s = 0; s < sampleCount; s++)
      {

        // For next samples


        if(light.type==0)
        {
          shadowRayDirection = PointOnSphere(inRayPayload.seed,light.position,1.0f);
        }
        else
        {
          shadowRayDirection = PointOnSphere(inRayPayload.seed,light.dir*distance,1.0f);
        }

        Shadow(responce, maxRecursion, shadowRayOrigin, shadowRayDirection);



        if (responce == 0)
        {
          litSamples++;
          processedLightColor += rayPayload.colour.rgb;
        }
      }


      if(litSamples>0)
      {
        shadowPower = (1.0f/sampleCount) * litSamples;
        processedLightColor /= litSamples;
      }



    }
    else
    {
      vec3 shadowRayDirection = l;

      Shadow(responce, maxRecursion, shadowRayOrigin, shadowRayDirection);


      if (responce == 0)
      {
        shadowPower = 1.0f;
        litSamples = 1;
        processedLightColor = rayPayload.colour.rgb;
      }
    }

    


    



    if (litSamples > 0)
    {

      float rdist = 1.0f / length(l);


      // Normalizing light
      l *= rdist;

      // Calculate light intensity
      float li = light.intensity * rdist * rdist;
      // Lights color
      vec3 lc = processedLightColor;

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



      colour += PI * f * ((li * cavity) * lc) * ndotl * shadowPower;



    }
  }
}


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
  return normalize(modelMatrix * (invTangentMatrix * textureNormal)).xyz;




  /*vec3 textureNormal = normalize((2.0f * normalize(texture(textureSamplers[normalTextureId], UV).xyz)) - 1.0f);
  textureNormal.y = -textureNormal.y;
  return normalize(modelMatrix * normalize(modelNormal * textureNormal)).xyz;*/
  
}

/*

vec3 GenerateNormal(vec3 modelNormal,
  mat3 modelMatrix, int normalTextureId, inout vec2 UV)
{
  vec3 textureNormal = normalize((2.0f * normalize(texture(textureSamplers[normalTextureId], UV).xyz)) - 1.0f);
  textureNormal.y = -textureNormal.y;
  return normalize(modelMatrix * normalize(modelNormal * textureNormal)).xyz;
  
}
*/

vec3 GenerateTangent(vec3 normal)
{


  vec3 tangent;

  vec3 c1 = cross(normal, vec3(0.0, 0.0, 1.0));
  vec3 c2 = cross(normal, vec3(0.0, 1.0, 0.0));

  if( length(c1) > length(c2) )
  {
  tangent = c1;
  }
  else
  {
  tangent = c2;
  }

  return normalize(tangent);
}


vec3 HitBarycentrics(vec3 a, vec3 b, vec3 c, vec3 bary)
{
  //return a + bary.x * (b - a) + bary.y * (c - a);
  return a * bary.x + b * bary.y + c * bary.z;
}

vec4 HitBarycentrics(vec4 a, vec4 b, vec4 c, vec3 bary)
{
  //return a + bary.x * (b - a) + bary.y * (c - a);
  return a * bary.x + b * bary.y + c * bary.z;
}

vec4 GenerateTangent(vec3 p1,vec3 p2,vec3 p3,vec2 u1,vec2 u2,vec2 u3, vec3 n1, vec3 n2, vec3 n3, vec3 barycentrics)
{

  float x1 = p2.x - p1.x;
  float x2 = p3.x - p1.x;
  float y1 = p2.y - p1.y;
  float y2 = p3.y - p1.y;
  float z1 = p2.z - p1.z;
  float z2 = p3.z - p1.z;


  float s1 = u2.x - u1.x;
  float s2 = u3.x - u1.x;
  float t1 = u2.y - u1.y;
  float t2 = u3.y - u1.y;

  float r = 1.0f / (s1 * t2 - s2 * t1);
  vec3 sdir = vec3((t2*x1-t1*x2)*r,(t2*y1-t1*y2)*r,(t2*z1-t1*z2)*r);
  vec3 tdir = vec3((s1*x2-s2*x1)*r,(s1*y2-s2*y1)*r,(s1*z2-s2*z1)*r);

  vec4 ta1 = vec4(
    normalize(sdir - n2 * dot(n2,sdir)),
    (dot(cross(n2,sdir),tdir)<0.0f?-1.0f:1.0f)
    );
  vec4 ta2 = vec4(
    normalize(sdir - n2 * dot(n2,sdir)),
    (dot(cross(n2,sdir),tdir)<0.0f?-1.0f:1.0f)
    );
  vec4 ta3 = vec4(
    normalize(sdir - n3 * dot(n3,sdir)),
    (dot(cross(n3,sdir),tdir)<0.0f?-1.0f:1.0f)
    );


  return HitBarycentrics(ta1, ta2, ta3, barycentrics);
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

