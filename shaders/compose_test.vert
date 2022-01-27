#version 450

layout (location = 1) out vec3 outColor;

const vec3 positions[4] = vec3[4](
    vec3(-1.f, 1.f, 0.0f),
   	vec3(-1.f,-1.f, 0.0f),
	vec3( 1.f,-1.f, 0.0f),
	vec3( 1.f, 1.f, 0.0f)
);

const vec3 color[4] = vec3[4](
    vec3(0.f, 0.f, 0.0f),
   	vec3(1.f, 0.f, 0.0f),
	vec3(0.f, 1.f, 0.0f),
	vec3(0.f, 0.f, 1.0f)
);

void main() 
{
    vec3 outColor = color[gl_VertexIndex];
	gl_Position =  vec4(positions[gl_VertexIndex], 1.0f);
}