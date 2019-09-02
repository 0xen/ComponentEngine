#version 460
#extension GL_NV_ray_tracing : require

struct RayPayload {
	vec3 color;
	float distance;
	vec3 normal;
	float reflector;
};

layout(location = 0) rayPayloadInNV float[4] rayPayload;

void main()
{


   	// View-independent background gradient to simulate a basic sky background
	const vec3 gradientStart = vec3(1.0, 0.6, 0.0);
	const vec3 gradientEnd = vec3(0.0, 0.6, 1.0);
	vec3 unitDir = normalize(gl_WorldRayDirectionNV);
	float t = 0.5 * (unitDir.y + 1.0);
	vec3 color = (1.0-t) * gradientEnd + t * gradientStart;
	rayPayload[0] = color.x;
	rayPayload[1] = color.y;
	rayPayload[2] = color.z;
}