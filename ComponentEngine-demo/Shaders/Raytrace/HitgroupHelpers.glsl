#include "Structures.glsl"

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

layout(location = 2) rayPayloadNV bool isShadowed;


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

uint MaxMaterialsPerModel = 8;

layout(binding = 4, set = 1) buffer MaterialsMapping { int map[]; }
materials_mapping;

layout(binding = 0, set = 2) uniform sampler2D[] textureSamplers;

layout(binding = 0, set = 3) buffer ModelPos { mat4 m[]; }
models;

layout(binding = 1, set = 3) buffer ModelOffsets { uint o[]; }
offsets;



layout (constant_id = 0) const uint SHADOW_MISS_SHADER_INDEX = 0;
layout (constant_id = 1) const uint MISS_SHADER_INDEX = 0;




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