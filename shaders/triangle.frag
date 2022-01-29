#version 450

//layout (location = 0) in vec4 in_world_pos;
layout (location = 0) in vec3 in_color;
//layout (location = 3) in vec2 in_tex_coord;
//layout (location = 2) in vec3 in_normal;


//layout(set = 1, binding = 0) uniform sampler2D _diffuse;
//layout(set = 1, binding = 1) uniform sampler2D _specular;
//layout(set = 1, binding = 2) uniform sampler2D _bump;


layout (location = 0) out vec4 outColor; //rgba

layout (location = 1) out vec4 outPosition; //xyz depth
layout (location = 2) out vec3 outNormal; //xyz
layout (location = 3) out vec3 outAlbedo; //rgb



void main()
{
	//outPosition = vec4(inWorldPos, 1.0);
	// Store linearized depth in alpha component
	//outPosition.a = gl_FragCoord.z;

	

	outPosition = vec4(0,1,0,1);//in_world_pos;
	outNormal = vec3(1,0,0);//in_normal;
	outAlbedo = in_color;
	


	// Write color attachments to avoid undefined behaviour (validation error)
	outColor = vec4(0,0,0, 1.0f); //vec4(1.f,0.f,0.f,1.0f);
}
