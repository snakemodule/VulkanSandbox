#version 450
#extension GL_KHR_vulkan_glsl : enable


layout(binding = 1) uniform ShadingBufferObject {
	vec4 specularColor;
	vec4 diffuseColor;
	vec4 ambientColor;
} shading;

layout (input_attachment_index = 0, set = 0, binding = 2) uniform subpassInput samplerPositionDepth;


layout (location = 0) in vec3 inColor;
layout (location = 1) in vec2 inTexCoord;

layout (location = 0) out vec4 outColor;

layout (constant_id = 0) const float NEAR_PLANE = 0.1f;
layout (constant_id = 1) const float FAR_PLANE = 10.0f;

float linearDepth(float depth)
{
	float z = depth * 2.0f - 1.0f; 
	return (2.0f * NEAR_PLANE * FAR_PLANE) / (FAR_PLANE + NEAR_PLANE - z * (FAR_PLANE - NEAR_PLANE));	
}

void main () 
{
	// Sample depth from deferred depth buffer and discard if obscured
	float depth = linearDepth(subpassLoad(samplerPositionDepth).a);
	
	vec4 check = {linearDepth(gl_FragCoord.z)/10.0f, depth/10.0f, 0.1f, 1.0f};

	//outColor = vec4(vec3(1.0f), 1.0f);//vec4(shading.diffuseColor.rgb, 0.5f);//texture(texSampler, inTexCoord);	
	//if (linearDepth(gl_FragCoord.z) < depth)
	//{
	//	discard;
	//	//outColor = vec4(1.0f, 0.0f, 0.0f, 1.0f);
	//	outColor = vec4(linearDepth(gl_FragCoord.z), depth, 0.0f, 1.0f);
	//} 
	//else 
	//{
		//discard;
		outColor = vec4(vec3(1.0f), 0.5f);//vec4(shading.diffuseColor.rgb, 0.5f);//texture(texSampler, inTexCoord);	
	//};
	//if ((depth != 0.0) && (linearDepth(gl_FragCoord.z) > depth))
	//{
	//	discard;
	//	//outColor = vec4(1.0f, 0.0f, 0.0f, 1.0f);
	//	//outColor = vec4(linearDepth(gl_FragCoord.z), depth, 0.0f, 1.0f);
	//} 
	//else 
	//{
		//discard;
		//outColor = check;
	//};

	//outColor = vec4(gl_FragCoord.z, depth, 1.0f, 1.0f);
}