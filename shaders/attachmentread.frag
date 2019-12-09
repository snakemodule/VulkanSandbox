#version 450
#extension GL_ARB_separate_shader_objects : enable

#extension GL_KHR_vulkan_glsl : enable

layout (input_attachment_index = 0, set = 0, binding = 0) uniform subpassInput inputColor;
layout (input_attachment_index = 1, set = 0, binding = 1) uniform subpassInput inputDepth;

//layout(binding = 1) uniform sampler2D texSampler;
//
//layout(binding = 2) uniform ShadingBufferObject {
//	//vec3 lightPosition;
//	//vec4 lightColor;
//	vec4 specularColor;
//	vec4 diffuseColor;
//	vec4 ambientColor;
//} shading;


//layout(location = 0) in vec3 fragColor;
//layout(location = 1) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

void main() {
	// Read color from previous color input attachment
	vec3 color = subpassLoad(inputColor).rgb;
	float depth = subpassLoad(inputDepth).r;	
	outColor = vec4(color, 1.0f);//brightnessContrast(color, ubo.brightnessContrast[0], ubo.brightnessContrast[1]);

	// Read depth from previous depth input attachment
	//float depth = subpassLoad(inputDepth).r;
	//outColor.rgb = vec3((depth - ubo.range[0]) * 1.0 / (ubo.range[1] - ubo.range[0]));


    //outColor = vec4(shading.diffuseColor.rgb, 1.0f);//texture(texSampler, fragTexCoord);
}