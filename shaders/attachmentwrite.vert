#version 450
#extension GL_ARB_separate_shader_objects : enable

//TODO split UBO for use with multiple models?
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



layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inNormal;
layout(location = 4) in ivec4 boneIndex;
layout(location = 5) in vec4 weight;


layout (location = 0) out vec3 outColor;
layout (location = 1) out vec3 outNormal;
layout (location = 2) out vec3 outViewVec;
layout (location = 3) out vec3 outLightVec;
layout (location = 4) out vec2 fragTexCoord;
layout (location = 5) out vec3 vertexColor;



void main() {
	mat4 BoneTransform  = ubo.boneTransforms[int(boneIndex[0])] * weight[0];
		 //BoneTransform += ubo.boneTransforms[int(boneIndex[1])] * weight[1];
		 //BoneTransform += ubo.boneTransforms[int(boneIndex[2])] * weight[2];
		 //BoneTransform += ubo.boneTransforms[int(boneIndex[3])] * weight[3];

    gl_Position = ubo.proj * ubo.view * ubo.model * BoneTransform * vec4(inPosition, 1.0);

	outColor = inColor;
	outNormal = inNormal;
    vertexColor = inColor;
    fragTexCoord = inTexCoord;

	outLightVec = vec3(0.0f, 5.0f, 15.0f) - inPosition;
	outViewVec = -inPosition.xyz;

}