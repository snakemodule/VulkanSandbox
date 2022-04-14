#version 450

layout (location = 0) in vec3 inColor;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec3 inWorldPos;
layout (location = 3) in vec2 fragTexCoord;


layout(set=1, binding = 0) uniform sampler2D diffuse;
layout(set=1, binding = 1) uniform sampler2D bump;
layout(set=1, binding = 2) uniform sampler2D specular;

layout (location = 0) out vec4 outColor;
layout (location = 1) out vec4 outPosition;
layout (location = 2) out vec4 outNormal;
layout (location = 3) out vec4 outAlbedo;



//layout (constant_id = 0) const float NEAR_PLANE = 0.1f;
//layout (constant_id = 1) const float FAR_PLANE = 10.0f;

//float linearDepth(float depth)
//{
//	float z = depth * 2.0f - 1.0f; 
//	return (2.0f * NEAR_PLANE * FAR_PLANE) / (FAR_PLANE + NEAR_PLANE - z * (FAR_PLANE - NEAR_PLANE));	
//}

void main() 
{
	outPosition = vec4(inWorldPos, 1.0);

	vec3 N = normalize(inNormal);
	//N.y = -N.y;
	outNormal = vec4(N, 1.0);

	//vec4 check = {linearDepth(gl_FragCoord.z)/FAR_PLANE, gl_FragCoord.z, 0.0f, 1.0f};
	outAlbedo = texture(diffuse, fragTexCoord);
    outAlbedo.a = texture(specular,fragTexCoord).r;

	// Store linearized depth in alpha component
	outPosition.a = gl_FragCoord.z;

	// Write color attachments to avoid undefined behaviour (validation error)
	outColor = vec4(1.0);
}