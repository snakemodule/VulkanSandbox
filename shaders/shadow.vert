#version 450

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inNormal;
layout(location = 4) in vec3 inTangent;
layout(location = 5) in vec3 inBitangent;

layout (location = 0) out vec4 outPos;
layout (location = 1) out vec3 outLightPos;

layout (binding = 0) uniform UBO 
{
	mat4 model;
	mat4 view; 
	mat4 projection;
} ubo;

layout(push_constant) uniform PushConsts 
{
	mat4 view;
} pushConsts;
 
out gl_PerVertex 
{
	vec4 gl_Position;
};
 
void main()
{
	gl_Position = ubo.projection * pushConsts.view * ubo.model * vec4(inPos, 1.0);
	//outPos = vec4(inPos, 1.0);	
	//outLightPos = ubo.lightPos.xyz; 
}