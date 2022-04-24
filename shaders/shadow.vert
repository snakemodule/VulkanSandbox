#version 450

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inNormal;
layout(location = 4) in vec3 inTangent;
layout(location = 5) in vec3 inBitangent;

layout (location = 0) out vec4 outPos;
layout (location = 1) out vec4 outLightPos;

layout (binding = 0) uniform UBO 
{
	mat4 model;
	mat4 view; 
	mat4 projection;
	vec4 lightPos;
} ubo;

layout(push_constant) uniform PushConsts 
{
	mat4 V;
} pushConsts;
 
out gl_PerVertex 
{
	vec4 gl_Position;
};
 
void main()
{	
	gl_Position = ubo.projection * pushConsts.V * ubo.model * vec4(inPos, 1.0);

	mat4 invView = inverse(ubo.view); // todo pre-prepare inverse matrix

	outPos = ubo.model * vec4(inPos, 1.0);	
	outLightPos = ubo.lightPos;
}