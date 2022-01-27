

//we will be using glsl version 4.5 syntax
#version 450

layout (location = 0) out vec3 outVertColor;
//layout (location = 1) out vec3 outNormal;
//layout (location = 2) out vec3 outWorldPos;
//layout (location = 3) out vec2 fragTexCoord;
//layout (location = 4) out vec3 vertexColor;

void main()
{
	//const array of positions for the triangle
	const vec3 positions[3] = vec3[3](
		vec3(1.f,1.f, 0.0f),
		vec3(-1.f,1.f, 0.0f),
		vec3(0.f,-1.f, 0.0f)
	);

	const vec3 vertexColor[3] = vec3[3](
		vec3(1.f, 0.f, 0.0f),
		vec3(0.f, 1.f, 0.0f),
		vec3(0.f, 0.f, 1.0f)
	);

	outVertColor = vertexColor[gl_VertexIndex];

	//output the position of each vertex
	gl_Position = vec4(positions[gl_VertexIndex], 1.0f);
}
