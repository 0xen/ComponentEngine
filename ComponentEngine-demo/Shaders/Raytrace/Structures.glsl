struct Camera
{
    mat4 view;
    mat4 proj;
    mat4 viewInverse;
    mat4 projInverse;

    vec3 maxRecursionDepthColor;
    uint gpuRecursionCount;
    uint recursionCount;
    float globalIlluminationBrightness;
    float globalIlluminationReflectionMissBrightness;

    // Camera Settings
    uint samplesPerFrame;
    float aperture;
    float focusDistance;
    float movementTolerance;
    uint dofSampleCount;
    uint mode;
};

struct RayPayload
{
  /* Color Sampling
  0: Got Color, Exit
  1: Shadow Test
  2: Hit Full Transparent
  3: Transparent Color
  4: Depth Test
  */
  /* Shadow
  0: Not Shadow
  1: Shadow
  */
  uint responce;
  uint recursion;
  vec4 colour;
  float depth;
  float power;

  vec3 origin;
  vec3 direction;
  uint seed;

	/*vec4 colour;
	uint recursion;
	uint randomSeed;
  bool depthTest;
  float depth;*/
};

struct Light
{
  vec3 position;
  float intensity;
  vec3 color;
  int alive;
  int type;
  vec3 dir;
  int modelID;
  float shadowRangeStartOffset;
  float shadowRangeEndOffset;
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
  int cavityTextureId;
  int aoTextureId;
  int heightTextureId;
};