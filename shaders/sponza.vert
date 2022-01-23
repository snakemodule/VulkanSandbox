#version 450

//vertex properties
layout (location = 0) in vec4 inPos;
layout (location = 1) in vec3 inColor;
layout (location = 2) in vec2 inTexCoord;
layout (location = 3) in vec3 inNormal;

layout(set = 0, binding = 0) uniform VP {
    mat4 view;
    mat4 proj;
} vp;

//layout(set = 2, binding = 0) uniform UniformBufferObject {
//    mat4 model;    
//} m;

//out
layout (location = 0) out vec3 outColor;
layout (location = 1) out vec3 outNormal;
layout (location = 2) out vec3 outWorldPos;
layout (location = 3) out vec2 fragTexCoord;

out gl_PerVertex
{
	vec4 gl_Position;
};

void main() 
{
	vec4 WorldPos = inPos;	
	gl_Position = vp.proj * vp.view * WorldPos;
		
	// TexCoord
	fragTexCoord = inTexCoord;

	// vertex color
	outColor = inColor;

	// Normal in world space
	mat3 mNormal = transpose(inverse(mat3(1.0)));
	outNormal = mNormal * normalize(inNormal);	

	// Vertex position in world space
	outWorldPos = vec3(WorldPos.xyz);
	// GL to Vulkan coord space
	outWorldPos.y = -outWorldPos.y;
	
}