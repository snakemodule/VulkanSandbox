#version 450

layout (location = 0) in vec4 inPos;
layout (location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inNormal;
layout(location = 4) in vec3 inTangent;
layout(location = 5) in vec3 inBitangent;

layout(set = 0, binding = 0) uniform matrixBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

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
    //passthrough
    fragTexCoord = inTexCoord;
	outColor = inColor;

    // Normal in world space
	mat3 mNormal = transpose(inverse(mat3(ubo.model)));
	outNormal = mNormal * normalize(inNormal);	

	vec4 WorldPos = ubo.model * inPos;

	// Vertex position in world space
	outWorldPos = vec3(WorldPos.xyz);
	// GL to Vulkan coord space
	//outWorldPos.y = -outWorldPos.y;

	gl_Position = ubo.proj * ubo.view * WorldPos;
	
}