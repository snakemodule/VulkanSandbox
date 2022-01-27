#version 450

//layout (input_attachment_index = 0, set = 0, binding = 0) uniform subpassInput samplerposition;
//layout (input_attachment_index = 1, set = 0, binding = 1) uniform subpassInput samplerNormal;
layout (input_attachment_index = 0, set = 0, binding = 0) uniform subpassInput samplerAlbedo;

layout (location = 1) in vec3 inColor;

layout (location = 0) out vec4 outColor;

void main() 
{
	// Read G-Buffer values from previous sub pass
	//vec3 fragPos = subpassLoad(samplerposition).rgb;
	//vec3 normal = subpassLoad(samplerNormal).rgb;
	vec4 albedo = subpassLoad(samplerAlbedo);
			
	// Ambient part
	//vec3 fragcolor  = albedo.rgb ;
  
	outColor=albedo;
}