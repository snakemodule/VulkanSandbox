#version 450

layout (input_attachment_index = 0, set = 0, binding = 0) uniform subpassInput samplerposition;
layout (input_attachment_index = 1, set = 0, binding = 1) uniform subpassInput samplerNormal;
layout (input_attachment_index = 2, set = 0, binding = 2) uniform subpassInput samplerAlbedo;

//layout (location = 0) in vec2 inUV;

layout (location = 0) out vec4 outColor;

layout (constant_id = 0) const int output_mode = 0;


//struct Light {
//	vec4 position;
//	vec3 color;
//	float radius;
//};
//
//layout (binding = 3) uniform UBO 
//{
//	vec4 viewPos;
//	Light lights[NUM_LIGHTS];
//} ubo;


void main() 
{
	// Read G-Buffer values from previous sub pass
	vec4 fragPos = subpassLoad(samplerposition);
	vec3 normal = subpassLoad(samplerNormal).rgb;
	vec4 albedo = subpassLoad(samplerAlbedo);
	
	#define ambient 0.15
	
	// Ambient part
	vec3 fragcolor  = albedo.rgb ;//* ambient;

	switch(output_mode) {
		case 0:
		outColor = vec4(fragcolor, 1.0);
		break;
		case 1:
		outColor = vec4(normal, 1.0);
		break;
		case 2:
		outColor = vec4(fragPos.rgb, 1.0); //pos
		break;
		case 3:
		outColor = vec4(fragPos.a); // depth
		break;
	}	
}