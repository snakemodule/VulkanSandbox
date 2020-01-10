#version 450

layout (location = 0) in vec4 inPos;
layout (location = 1) in vec3 inColor;
layout (location = 2) in vec3 inNormal;
layout (location = 3) in vec2 inTexCoord;
layout (location = 4) in ivec4 boneIndex;
layout (location = 5) in vec4 weight;


layout(binding = 0) uniform UniformBufferObject {
	//vec3 lightPosition;
	//vec4 lightColor;
	//vec4 specularColor;
	//vec4 diffuseColor;
	//vec4 ambientColor;
    mat4 model;
    mat4 view;
    mat4 proj;
	mat4 boneTransforms[52]; //TODO set maximum number of bones
} ubo;

layout (location = 0) out vec3 outColor;
layout (location = 1) out vec2 outTexCoord;
//layout (location = 2) out vec3 outNormal;
//layout (location = 3) out vec3 vertexColor;


void main () 
{
	outColor = inColor;
	outTexCoord = inTexCoord;

	mat4 BoneTransform  = ubo.boneTransforms[int(boneIndex[0])] * weight[0];
		 //BoneTransform += ubo.boneTransforms[int(boneIndex[1])] * weight[1];
		 //BoneTransform += ubo.boneTransforms[int(boneIndex[2])] * weight[2];
		 //BoneTransform += ubo.boneTransforms[int(boneIndex[3])] * weight[3];
	
	gl_Position = ubo.proj * ubo.view * ubo.model * BoneTransform * vec4(inPos.xyz, 1.0);		
}