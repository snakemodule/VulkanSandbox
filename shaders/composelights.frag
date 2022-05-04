#version 450

layout (input_attachment_index = 0, set = 0, binding = 0) uniform subpassInput samplerposition;
layout (input_attachment_index = 1, set = 0, binding = 1) uniform subpassInput samplerNormal;
layout (input_attachment_index = 2, set = 0, binding = 2) uniform subpassInput samplerAlbedo;
layout (set = 0, binding = 3) uniform screenToView {
    mat4 inverseProjection;
    uvec4 tileSizes;
    uvec2 screenDimensions;
	float sliceScalingFactor;
	float sliceBiasFactor;
};
layout (set = 0, binding = 4) uniform Camera {
	vec4 cameraPos_wS;
};
layout(set = 0, binding = 5) uniform matrixBufferObject {
    mat4 model;
    mat4 viewMatrix;
    mat4 proj;
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
//layout (set = 1, binding = 3) uniform samplerCube shadowCubeMap;
//------------------------------------------------------------
layout (set = 2, binding = 0) uniform samplerCube shadowCubeMap;
layout (set = 2, binding = 1) uniform ubo {
    mat4 cameraRotation;
    uvec2 screenDimensions2; 
    float yfovRad;
};
//-----------------------------

layout (location = 0) out vec4 outColor;

layout (constant_id = 0) const int output_mode = 0;

#define EPSILON 0.00
const float zNear = 0.1f;
const float zFar = 10.f;

float linearDepth(float depthSample);
uint getDepthSlice(float depth);
vec3 calcPointLight(uint index, vec3 normal, vec3 fragPos, vec3 viewDir, 
					vec3 col, float spec, float viewDistance);


vec3 color_int(uint i) 
{
	uint m = i%8;
	//return vec3(m|1, (m>>1)|1, (m>>2) | 1);
	switch(m) {
		case 0:
			return vec3(0,0,0);
		case 1:
			return vec3(1,0,0);
		case 2:
			return vec3(0,1,0);
		case 3:
			return vec3(0,0,1);
		case 4:
			return vec3(1,0,1);
		case 5:
			return vec3(0,1,1);
		case 6:
			return vec3(1,1,0);
		case 7:
			return vec3(1,1,1);
	}	
}

vec4 clipToView(vec4 clip)
{
    //View space transform
    vec4 view = inverseProjection * clip;

    //Perspective projection
    view = view / view.w;
    
    return view;
}

vec4 screen2View(vec4 screen){
    //Convert to NDC
    vec2 texCoord = screen.xy / screenDimensions.xy;

    //Convert to clipSpace
    // vec4 clip = vec4(vec2(texCoord.x, 1.0 - texCoord.y)* 2.0 - 1.0, screen.z, screen.w);
    vec4 clip = vec4(vec2(texCoord.x, texCoord.y)* 2.0 - 1.0, screen.z, screen.w);
    //Not sure which of the two it is just yet

    return clipToView(clip);
}



void main() 
{
	// Read G-Buffer values from previous sub pass
	vec4 fragPos = subpassLoad(samplerposition);
	vec3 normal = subpassLoad(samplerNormal).rgb;
	vec4 albedo = subpassLoad(samplerAlbedo);
	
	#define ambient 0.03
	vec3 fragcolor  = albedo.rgb * ambient;
	
	vec3 viewDir   = normalize(cameraPos_wS.xyz - fragPos.xyz);
	float viewDistance = length(cameraPos_wS.xyz - fragPos.xyz);
	
	//Locating which cluster you are a part of
	vec4 fragPos_vs = viewMatrix * fragPos;
    uint zTile     = getDepthSlice(-fragPos_vs.z); 	
    uvec3 tiles    = uvec3( uvec2( gl_FragCoord.xy / tileSizes[3] ), zTile);
    uint tileIndex = tiles.x +
                     tileSizes.x * tiles.y +
                     (tileSizes.x * tileSizes.y) * tiles.z;

	vec3 zColor = color_int(zTile);
		
	vec3 tileColor = zColor;
	tileColor = ((tiles.x % 2 == 0 && tiles.y % 2 == 1) 
				|| (tiles.x % 2 == 1 && tiles.y % 2 == 0)) ? color_int(zTile) : vec3(1)-color_int(zTile);
	// Point lights
    uint lightCount       = lightGrid[tileIndex].count;
    uint lightIndexOffset = lightGrid[tileIndex].offset;


	uint myLightCount = 0;

	// Shadow
	/*
	
	*/
	float shadow =1;
    

    if (output_mode == 0) 
    {
        vec3 fragToLight = fragPos.xyz - pointLight[0].position.xyz; 
        float closestDepth = texture(shadowCubeMap, fragToLight).r;
        float currentDepth = length(fragToLight); 
        float bias = 0.05; 
        float shadow = currentDepth - bias > closestDepth ? 0.0 : 1.0; 
        fragcolor += shadow * calcPointLight(0, normal, fragPos.xyz, viewDir, albedo.rgb, albedo.a, viewDistance);
    } 
    else 
    {
        for(uint i = 0; i < lightCount; i++)
        {
            uint index = globalLightIndexList[lightIndexOffset + i];
            if(index != 0) {
                myLightCount++;
                fragcolor += shadow * calcPointLight(index, normal, fragPos.xyz, viewDir, albedo.rgb, albedo.a, viewDistance);
            }
        }
    }
    	
	switch(output_mode) {
		case 0:		
		    break;
		case 1:			
			float aspect = screenDimensions.x / screenDimensions.y;   
            vec2 uv = gl_FragCoord.xy / screenDimensions.xy;
            uv = uv * 2.0 - 1.0;
            uv.x *= aspect;
            uv.x *= -1;
            uv *= tan(yfovRad/2);
            vec3 ray_direction = normalize(vec3(uv, 1.0));
            ray_direction = (cameraRotation * vec4(ray_direction, 0)).xyz;
            vec4 rayColor = vec4(texture(shadowCubeMap, ray_direction).xxx,1);
            //rayColor *= vec4(ray_direction, 1);			
			fragcolor = vec3(rayColor/zFar);
		    break;
		case 2:
		    break;
		case 3:		
            if (myLightCount>0)  
			    fragcolor = vec3(color_int(myLightCount));            
		    break;
	}	
    outColor = vec4(fragcolor, 1.0);
}

float linearDepth(float depth)    
{
    return zNear * zFar / (zFar + depth * (zNear - zFar));
}

/*
float linearDepth(float depth)
{
	float z = depth * 2.0f - 1.0f; 
	return (2.0f * zNear * zFar) / (zFar + zNear - z * (zFar - zNear));	
}
*/


uint getDepthSlice(float viewZ) 
{
    uint numSlices = tileSizes[2];    
	const float numerator = numSlices * log(viewZ/zNear);
	const float denominator = log(zFar/zNear);
	return uint(floor(numerator/denominator));
}

vec3 calcPointLight(uint index, vec3 normal, vec3 fragPos, 
	vec3 viewDir, vec3 col, float spec, float viewDistance)
{
    vec3 position = pointLight[index].position.xyz;
    vec3 color    = pointLight[index].color.rgb;
	float radius  = pointLight[index].range;
    //Attenuation calculation that is applied to all
    float distance = length(position - fragPos);
    //float attenuation = 1.0 / (1.0 + 0.07 * distance + 0.017 * (distance * distance));
	float attenuation = pow(clamp(1 - pow((distance / radius), 4.0), 0.0, 1.0), 2.0)/(1.0  + (distance * distance) );

    //ambient component
    // vec3 ambient = 0.005 * col;
    //float ambient = 0.005;

    //diffuse component
    vec3 lightDir = normalize(position - fragPos);
    float nDotL   = clamp(dot(lightDir, normal), 0.0, 1.0);
    float diffuse  = nDotL ;

    //specular component
    vec3 halfway  = normalize(lightDir + viewDir);
    float nDotHBP = pow(max(dot(normal, halfway), 0.0), 128.0); //N dot H using blinn phong
    float specular = nDotHBP * spec;
	specular = 0;

    // //shadow stuff
    //vec3 fragToLight = fragPos - position;
    //float shadow = calcPointLightShadows(depthMaps[index], fragToLight, viewDistance);
    
    //total contibution 
    // return  attenuation * (ambient + (1.0 - shadow) * (diffuse + specular));
    return  attenuation * (color * (diffuse + specular)) * col;
	//return vec3(1.0, 0.0,0.0);
	
}




/*vec3 calcPointLight(uint index, vec3 normal, vec3 fragPos,
                    vec3 viewDir, vec3 albedo, float rough,
                    float metal, vec3 F0,  float viewDistance){
    //Point light basics
    vec3 position = pointLight[index].position.xyz;
    vec3 color    = 100.0 * pointLight[index].color.rgb;
    float radius  = pointLight[index].range;

    //Stuff common to the BRDF subfunctions 
    vec3 lightDir = normalize(position - fragPos);
    vec3 halfway  = normalize(lightDir + viewDir);
    float nDotV = max(dot(normal, viewDir), 0.0);
    float nDotL = max(dot(normal, lightDir), 0.0);

    //Attenuation calculation that is applied to all
    float distance    = length(position - fragPos);
    float attenuation = pow(clamp(1 - pow((distance / radius), 4.0), 0.0, 1.0), 2.0)/(1.0  + (distance * distance) );
    vec3 radianceIn   = color * attenuation;

    //Cook-Torrance BRDF
    float NDF = distributionGGX(normal, halfway, rough);
    float G   = geometrySmith(nDotV, nDotL, rough);
    //vec3  F   = fresnelSchlick(max(dot(halfway,viewDir), 0.0), F0);

    //Finding specular and diffuse component
    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metal;

    vec3 numerator = NDF * G * F;
    float denominator = 4.0 * nDotV * nDotL;
    vec3 specular = numerator / max(denominator, 0.0000001);
    // vec3 specular = numerator / denominator;

    vec3 radiance = (kD * (albedo / M_PI) + specular ) * radianceIn * nDotL;

    //shadow stuff
    vec3 fragToLight = fragPos - position;
    float shadow = calcPointLightShadows(depthMaps[index], fragToLight, viewDistance);
    
    radiance *= (1.0 - shadow);

    return radiance;
}*/



