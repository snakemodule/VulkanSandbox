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

struct VolumeTileAABB{
    vec4 minPoint;
    vec4 maxPoint;
};

layout (set = 1, binding = 3) buffer clusterAABB {
    VolumeTileAABB cluster[ ];
};

//------------------------------------------------------------

layout (location = 0) out vec4 outColor;

layout (constant_id = 0) const int output_mode = 0;


const float zNear = 0.1f;
const float zFar = 10.f;

float linearDepth(float depthSample);
uint getDepthSlice(float depth);
vec3 calcPointLight(uint index, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 col, float spec, float viewDistance);



bool insideLightRadius(uint index, vec3 pos);

bool testSphereAABB(float radius, vec3 center, uint tileIndex);
float sqDistPointAABB(vec3 point, uint tile);

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

//todo invert this into z index from depth.
uint verifyCluster(float depth)//uint zIndex) 
{
	//Eye position is zero in view space
    const vec3 eyePos = vec3(0.0);
    
	round(depth);
	float num = log(depth/zNear)*float(tileSizes[2]);
	float denom = log(zFar/zNear);
    uint zIdx = uint(round(num/denom));
    //Near and far values of the cluster in view space
    return zIdx;//zNear * pow(zFar/zNear, zIndex/float(tileSizes[2]));
    //float tileFar   = -zNear * pow(zFar/zNear, (gl_WorkGroupID.z + 1)/float(gl_NumWorkGroups.z));	
}

bool insideBox3D(vec3 v, vec3 bottomLeft, vec3 topRight) {
    vec3 s = step(bottomLeft, v) - step(topRight, v);
    return bool(s.x * s.y * s.z); 
}

void main() 
{
	// Read G-Buffer values from previous sub pass
	vec4 fragPos = subpassLoad(samplerposition);
	vec3 normal = subpassLoad(samplerNormal).rgb;
	vec4 albedo = subpassLoad(samplerAlbedo);
	
	#define ambient 0.05
	vec3 fragcolor  = albedo.rgb * ambient;
	
	vec3 viewDir   = normalize(cameraPos_wS.xyz - fragPos.xyz);
	float viewDistance = length(cameraPos_wS.xyz - fragPos.xyz);
	
	//Locating which cluster you are a part of
	vec4 fragPos_vs = viewMatrix * fragPos;
    uint zTile     = getDepthSlice(-fragPos_vs.z); //uint(max(log2(linearDepth(gl_FragCoord.z)) * scale + bias, 0));
	//uint zTile = uint(max(log2(linearDepth(fragPos.a)) * sliceScalingFactor + sliceBiasFactor, 0.0));
    uvec3 tiles    = uvec3( uvec2( gl_FragCoord.xy / tileSizes[3] ), zTile);
    uint tileIndex = tiles.x +
                     tileSizes.x * tiles.y +
                     (tileSizes.x * tileSizes.y) * tiles.z;

	vec3 zColor = color_int(zTile);
	



	//if(round(cluster[tileIndex].maxPoint[3]) == 666){
	//if(tiles.z < 24){
		//fragcolor=vec3(0,1,1);
		
	//} 
	//if (tiles.x % 2 == 1) {
	//	fragcolor=vec3(0,0.5,.5);
	//}

	if(zTile==0 || !(tileIndex<cluster.length())) {
		//fragcolor=vec3(1.,0,0.5);
	} else {
		//fragcolor = zColor;
	}
	
	


	vec3 tileColor = zColor;
	tileColor = ((tiles.x % 2 == 0 && tiles.y % 2 == 1) 
				|| (tiles.x % 2 == 1 && tiles.y % 2 == 0)) ? color_int(zTile) : vec3(1)-color_int(zTile);
	
	
	// Point lights
    uint lightCount       = lightGrid[tileIndex].count;
    uint lightIndexOffset = lightGrid[tileIndex].offset;

	uint myLightCount = 0;
	
	//loop lights in this cluster
	for(uint i = 0; i < lightCount; i++)
	//for(uint i = 0; i < lightCount; i++)
	{
		

	
        uint index = globalLightIndexList[lightIndexOffset + i];
		//if(insideLightRadius(index, fragPos.xyz)) {
		//	myLightCount++;
			//fragcolor+= vec3(1,0,0);
		//}
		/*// To light
		vec3 lightPos = pointLight[index].position.xyz;
		vec3 L = lightPos - fragPos.xyz;
		float dist = length(L);
		L = normalize(L);

		// To viewer
		vec3 viewPos = cameraPos_wS.xyz;//vec3(ubo.view * ubo.model * vec4(ubo.viewPos.xyz, 1.0));
		vec3 V = viewPos - fragPos.xyz;
		V = normalize(V);

		// Attenuation		
		float atten = pointLight[index].range / (pow(dist, 2.0) + 1.0);

		// Diffuse part
		vec3 N = normalize(normal);
		float NdotL = max(0.0, dot(N, L));
		vec3 diff = pointLight[index].color.rgb * albedo.rgb * NdotL * atten;

		// Specular part
		vec3 R = reflect(-L, N);
		float NdotR = max(0.0, dot(R, V));
		vec3 spec = pointLight[index].color.rgb * albedo.a * pow(NdotR, 16.0) * (atten * 1.5);

		//fragcolor += diff + spec;*/
        fragcolor += calcPointLight(index, normal, fragPos.xyz, viewDir, albedo.rgb, albedo.a, viewDistance);		
    }
	
	/*for(uint x = 0; x < tileSizes.x; x++) 
	{
		for(uint y = 0; y < tileSizes.y; y++) 
		{
			uint index = x + 
						tileSizes.x * y +
                     	(tileSizes.x * tileSizes.y) * 15;
			VolumeTileAABB currentCell = cluster[index];
			if( insideBox3D(fragPos_vs.xyz, currentCell.minPoint.xyz, currentCell.maxPoint.xyz))
			{
				fragcolor = vec3(0.2863, 0.0, 0.0);
			}
		}
		
	}
	
	if(zTile == 15 && tiles.x % 2 == 1){
		fragcolor = vec3(0.9804, 0.0, 0.0);
	}*/

	//if(testSphereAABB(1, vec3(0,0,-2), tileIndex)) 	{
			//fragcolor = vec3(1.0, 1.0, 1.0);			
	//};

	switch(output_mode) {
		case 0:
		outColor = vec4(fragcolor, 1.0);		
		break;
		case 1:
		//outColor = vec4(normal, 1.0);
		fragcolor += color_int(lightCount);
		outColor = vec4(fragcolor, 1.0);		
		break;
		case 2:
		fragcolor = mix(color_int(lightCount), fragcolor, 0.5);	
		//fragcolor += color_int(uint(max(log2(linearDepth(fragPos.a)) * sliceScalingFactor + sliceBiasFactor, 0.0)));
		outColor = vec4(fragcolor, 1.0);				
		break;
		case 3:
		//if(tiles.z > 0) {
		//	fragcolor = vec3(1);
		//}
		fragcolor = color_int(myLightCount);
		outColor = vec4(fragcolor, 1.0);				
		break;
	}	
}

float linearDepth(float depth)    
{
    return zNear * zFar / (zFar + depth * (zNear - zFar));
}

uint getDepthSlice(float viewZ) 
{
    uint numSlices = tileSizes[2];    
	const float numerator = numSlices * log(viewZ/zNear);
	const float denominator = log(zFar/zNear);
	return uint(floor(numerator/denominator));
}

bool insideLightRadius(uint index, vec3 pos) 
{
	vec3 light_position = pointLight[index].position.xyz;
	float range = pointLight[index].range;	
	return distance(pos, light_position) <= range;
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

bool testSphereAABB(float radius, vec3 center, uint tileIndex){
    //float radius = pointLight[light].range;
    //vec3 center  = vec3(viewMatrix * pointLight[light].position);
    float squaredDistance = sqDistPointAABB(center, tileIndex);

    return squaredDistance <= (radius * radius);
}

float sqDistPointAABB(vec3 point, uint tileIndex){
    float sqDist = 0.0;
    VolumeTileAABB currentCell = cluster[tileIndex];
    for(int i = 0; i < 3; ++i){
        float v = point[i];
        if(v < currentCell.minPoint[i]){
            sqDist += (currentCell.minPoint[i] - v) * (currentCell.minPoint[i] - v);
        }
        if(v > currentCell.maxPoint[i]){
            sqDist += (v - currentCell.maxPoint[i]) * (v - currentCell.maxPoint[i]);
        }
    }

    return sqDist;
}

