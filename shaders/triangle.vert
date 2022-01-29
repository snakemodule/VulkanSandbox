#version 450

//layout(location = 0) in vec4 vert_pos;
//layout(location = 1) in vec3 vert_color;
//layout(location = 2) in vec2 tex_coord;
//layout(location = 3) in vec3 vert_normal;

//layout(set = 0, binding = 0) uniform VP {
//    mat4 view;
//    mat4 proj;
//} vp;

//layout( push_constant ) uniform M
//{
//	mat4 model;	
//} push;

//layout (location = 0) out vec4 out_world_pos;
layout (location = 0) out vec3 out_color;
//layout (location = 3) out vec2 out_tex_coord;
//layout (location = 2) out vec3 out_normal;



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

	out_color = vertexColor[gl_VertexIndex];
	gl_Position = vec4(positions[gl_VertexIndex], 1.0);

	//vec4 world_position = push.model * vert_pos;
	//vec4 final_position = vp.proj * vp.view * world_position;

	//out_world_pos = world_position;
	//out_color = vert_color;
	//out_tex_coord = tex_coord;
	//out_normal = vert_normal;

	//output the position of each vertex
	//gl_Position = final_position;
}
