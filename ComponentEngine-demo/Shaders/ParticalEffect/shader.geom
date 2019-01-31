#version 440

layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

layout(location = 0) in vec4 inColor[1];
layout(location = 1) in float inScale[1];


layout(location = 0) out vec4 outColor;
void main() {    
	outColor = inColor[0];
	float scale = inScale[0];

    vec4 position = gl_in[0].gl_Position;
	
    gl_Position = position + (vec4(-0.5f, -0.5f, 0.0f, 0.0f) * scale);
    EmitVertex();
    gl_Position = position + (vec4(0.5f, -0.5f, 0.0f, 0.0f) * scale);
    EmitVertex();
    gl_Position = position + (vec4(-0.5f, 0.5f, 0.0f, 0.0f) * scale);
    EmitVertex();
    gl_Position = position + (vec4(0.5f, 0.5f, 0.0f, 0.0f) * scale);
    EmitVertex();


    EndPrimitive();

}  