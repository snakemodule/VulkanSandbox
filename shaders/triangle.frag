#version 450
#extension GL_KHR_vulkan_glsl : enable

layout (location = 0) in vec3 inVertexColor;


//layout (location = 1) in vec3 inNormal;

//layout (location = 3) in vec2 fragTexCoord;
//layout (location = 4) in vec3 vertexColor;

layout (location = 0) out vec4 outColor;
layout (location = 1) out vec4 outAlbedo;

//layout (location = 1) out vec4 outPosition;
//layout (location = 2) out vec4 outNormal;
//layout (location = 3) out vec4 outAlbedo;



void main()
{
	//outPosition = vec4(inWorldPos, 1.0);
	// Store linearized depth in alpha component
	//outPosition.a = gl_FragCoord.z;

	outAlbedo.rgb = vec3(inVertexColor);

	//outNormal = vec4(1.0);


	// Write color attachments to avoid undefined behaviour (validation error)
	outColor = vec4(inVertexColor, 1.0f);//vec4(1.f,0.f,0.f,1.0f);
}
