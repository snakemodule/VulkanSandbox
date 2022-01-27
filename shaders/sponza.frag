#version 450



//interpolated
layout (location = 0) in vec3 inColor;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec3 inWorldPos;
layout (location = 3) in vec2 fragTexCoord;

//layout(set = 1, binding = 0) uniform sampler2D _diffuse;
//layout(set = 1, binding = 1) uniform sampler2D _specular;
//layout(set = 1, binding = 2) uniform sampler2D _bump;

layout (location = 0) out vec4 outColor;
layout (location = 1) out vec4 outPosition;
layout (location = 2) out vec4 outNormal;
layout (location = 3) out vec4 outAlbedo;

layout (constant_id = 0) const int ENABLE_DISCARD = 0;
//layout (constant_id = 0) const float NEAR_PLANE = 0.1f;
//layout (constant_id = 1) const float FAR_PLANE = 64.0f;


void main() 
{
	outPosition = vec4(inWorldPos, gl_FragCoord.z);

	//vec4 color = texture(_diffuse, fragTexCoord);
	vec4 color = vec4(1.0f);	
	outAlbedo.rgb = color.rgb;

	if (ENABLE_DISCARD == 0)
	{
		//vec3 B = normalize(inBitangent);
		//vec3 N = normalize(inNormal);
		//vec3 T = normalize(inTangent);
		//mat3 TBN = mat3(T, B, N);
		//vec3 nm = texture(samplerNormal, inUV).xyz * 2.0 - vec3(1.0);
		//nm = TBN * normalize(nm);
		//outNormal = vec4(nm * 0.5 + 0.5, 0.0);
	}
	else
	{
		//outNormal = vec4(normalize(inNormal) * 0.5 + 0.5, 0.0);
		if (color.a < 0.5)
		{
			discard;
		}
	}

	vec3 N = normalize(inNormal);
	N.y = -N.y;
	outNormal = vec4(N, 1.0);

	// Write color attachments to avoid undefined behaviour (validation error)
	outColor = vec4(0.0);
}