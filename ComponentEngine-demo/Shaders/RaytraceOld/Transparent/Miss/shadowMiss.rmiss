#version 460
#extension GL_NV_ray_tracing : require

layout(location = 0) rayPayloadInNV float[4] isShadowed;

void main()
{
  isShadowed[0] = 0.0f;
}