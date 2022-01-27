#version 450

layout(location = 0) in vec4 inPos;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inNormal;
layout(location = 4) in ivec4 boneIndex;
layout(location = 5) in vec4 weight;

//layout(binding = 0) uniform UniformBufferObject {
//	//vec3 lightPosition;
//	//vec4 lightColor;
//	//vec4 specularColor;
//	//vec4 diffuseColor;
//	//vec4 ambientColor;
//    mat4 model;
//    mat4 view;
//    mat4 proj;
//	mat4 boneTransforms[52]; //TODO set maximum number of bones
//} ubo;

layout (location = 0) out vec3 outColor;
layout (location = 1) out vec3 outNormal;
layout (location = 2) out vec3 outWorldPos;
layout (location = 3) out vec2 fragTexCoord;
layout (location = 4) out vec3 vertexColor;

out gl_PerVertex
{
	vec4 gl_Position;
};

void main() 
{
	//mat4 BoneTransform  = ubo.boneTransforms[int(boneIndex[0])] * weight[0];
		 //BoneTransform += ubo.boneTransforms[int(boneIndex[1])] * weight[1];
		 //BoneTransform += ubo.boneTransforms[int(boneIndex[2])] * weight[2];
		 //BoneTransform += ubo.boneTransforms[int(boneIndex[3])] * weight[3];

	//vec4 WorldPos = ubo.model * BoneTransform * inPos;

	//gl_Position = ubo.proj * ubo.view * WorldPos;
	gl_Position = WorldPos;
	
	// Currently just vertex color
	outColor = inColor;

	// Normal in world space
	//mat3 mNormal = transpose(inverse(mat3(ubo.model)));
	outNormal = mNormal * normalize(inNormal);

	// Vertex position in world space
	outWorldPos = vec3(WorldPos.xyz);
	// GL to Vulkan coord space
	outWorldPos.y = -outWorldPos.y;
	
}