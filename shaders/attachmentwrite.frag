#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 1) uniform sampler2D texSampler;

layout(binding = 2) uniform ShadingBufferObject {
	vec4 specularColor;
	vec4 diffuseColor;
	vec4 ambientColor;
} shading;

layout (location = 0) in vec3 inColor;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec3 inViewVec;
layout (location = 3) in vec3 inLightVec;
layout (location = 4) in vec2 texCoord;
layout (location = 5) in vec3 vertexColor;



layout(location = 0) out vec4 outColor;



void main() {
    outColor = vec4(inNormal, 1.0f);//texture(texSampler, fragTexCoord); //vec4(shading.diffuseColor.rgb, 1.0f);//
}