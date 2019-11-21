struct Camera
{
    mat4 view;
    mat4 proj;
    mat4 viewInverse;
    mat4 projInverse;

	uint recursionCount;
	// Camera Settings
	uint sampleCount;
	float aperture;
	float focusDistance;
};

struct RayPayload
{
	vec4 colourAndDistance; // Color.xyz + distance
	uint randomSeed;
};

struct Light
{
  vec3 position;
  float intensity;
  vec3 color;
  int alive;
  int type;
  vec3 dir;
};

struct Offsets
{
  uint index;
  uint vertex;
  uint position;
};

struct Vertex
{
  vec3 pos;
  vec3 nrm;
  vec3 color;
  vec2 texCoord;
  int matIndex;
};

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