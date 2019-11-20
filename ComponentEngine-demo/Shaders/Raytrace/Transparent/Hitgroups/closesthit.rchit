#version 460
#extension GL_NV_ray_tracing : require
#extension GL_EXT_nonuniform_qualifier : enable

layout(location = 0) rayPayloadInNV float[4] inRayPayload;

//layout(location = 1) rayPayloadNV bool isShadowed;

layout(location = 1) rayPayloadNV float[4] rayPayload;



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



struct Light
{
  vec3 position;
  float intensity;
  vec3 color;
  int alive;
  int type;
  vec3 dir;
};

Light unpackLight(uint index)
{
  uint startingIndex = index * 3;

  vec4 d0 = lights.l[startingIndex];
  vec4 d1 = lights.l[startingIndex + 1];
  vec4 d2 = lights.l[startingIndex + 2];

  Light l;
  l.position = vec3(d0.x,d0.y,d0.z);
  l.intensity = d0.w;
  l.color = vec3(d1.z,d1.y,d1.x);
  l.alive = floatBitsToInt(d1.w);
  l.type = floatBitsToInt(d2.x);
  l.dir = vec3(d2.y,d2.z,d2.w);

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
  int normalTextureId;
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
  m.normalTextureId = floatBitsToInt(d5.b);

  return m;
}

void main()
{
  float PI = 3.14159265359f;
  float GAMMA = 2.2f;


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



  // To do, global illumination /////////


    vec3 colour = vec3(0.0f,0.0f,0.1f);

    int currentResursion = floatBitsToInt(inRayPayload[3]);

  if(currentResursion>0)
  {
    currentResursion -= 1;
    rayPayload[3] = intBitsToFloat(currentResursion);



    float refractiveIndex = 1.5;
    vec3 refrac = refract(normalize(viewVector), 
               normalize(normal), 1.0 / refractiveIndex);


    traceNV(topLevelAS, gl_RayFlagsOpaqueNV | gl_RayFlagsCullBackFacingTrianglesNV , 0xff, 0, 0, 0, origin, 0.01 refrac, 1000.0, 1);

    colour += vec3(rayPayload[0], rayPayload[1], rayPayload[2]);
    inRayPayload[3] = intBitsToFloat(currentResursion);
  }
  

  inRayPayload[0] = colour.x;
  inRayPayload[1] = colour.y;
  inRayPayload[2] = colour.z;


}


