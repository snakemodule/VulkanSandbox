#version 450

layout (input_attachment_index = 0, set = 0, binding = 0) uniform subpassInput samplerposition;
layout (input_attachment_index = 1, set = 0, binding = 1) uniform subpassInput samplerNormal;
layout (input_attachment_index = 2, set = 0, binding = 2) uniform subpassInput samplerAlbedo;
layout (set = 0, binding = 3) uniform Camera {
	vec4 cameraPos_wS;
};
//------------------------------------------------------------
struct PointLight 
{
    vec4 position;
    vec4 color;
    uint enabled;
    float intensity;
    float range;
};
layout (set = 1, binding = 0) buffer lightSSBO { PointLight pointLight[]; };
layout (set = 1, binding = 1) buffer lightIndexSSBO { uint globalLightIndexList[]; };
struct LightGrid { uint offset; uint count; };
layout (set = 1, binding = 2) buffer lightGridSSBO { LightGrid lightGrid[]; };
//------------------------------------------------------------
layout (set = 2, binding = 0) uniform samplerCube shadowCubeMap;
//-----------------------------

layout (location = 0) out vec4 outColor;

layout (constant_id = 0) const int output_mode = 0;


const float zNear = 0.1f;
const float zFar = 10.f;


vec3 calcPointLight(uint index, vec3 normal, vec3 fragPos, vec3 viewDir, 
					vec3 col, float spec, float viewDistance);

void main() 
{
	// Read G-Buffer values from previous sub pass
	vec4 fragPos = subpassLoad(samplerposition);
	vec3 normal = subpassLoad(samplerNormal).rgb;
	vec4 albedo = subpassLoad(samplerAlbedo);
	
	vec3 fragcolor  = albedo.rgb * 0.03;
	
	vec3 viewDir   = normalize(cameraPos_wS.xyz - fragPos.xyz);
	float viewDistance = length(cameraPos_wS.xyz - fragPos.xyz);
	
	vec3 fragToLight = fragPos.xyz - pointLight[0].position.xyz; 

	vec3 sampleVec = fragToLight;
	sampleVec.z *= -1; //fix light-depth cam so we don't need this	

    float closestDepth = texture(shadowCubeMap, sampleVec).r;
	float currentDepth = length(fragToLight); 
    float bias = 0.0; //todo: tune bias
	float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;

	
    
    fragcolor += shadow * calcPointLight(0, normal, fragPos.xyz, viewDir, albedo.rgb, albedo.a, viewDistance);		
    

	switch(output_mode) {
		case 0:		
		outColor = vec4(vec3(closestDepth/zFar), 1.0);
		break;
		case 1:
		//...
		break;
		case 2:
		//...
		break;
		case 3:				
		//...
		break;
	}	
}






