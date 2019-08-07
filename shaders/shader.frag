#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 1) uniform sampler2D texSampler;

layout(binding = 2) uniform ShadingBufferObject {
	//vec3 lightPosition;
	//vec4 lightColor;
	vec4 specularColor;
	vec4 diffuseColor;
	vec4 ambientColor;
} shading;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = vec4(shading.diffuseColor.rgb, 1.0f);//texture(texSampler, fragTexCoord);
}