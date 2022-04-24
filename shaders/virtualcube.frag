#version 450 core

layout (set = 0, binding = 0) uniform samplerCube shadowCubeMap;
layout (set = 0, binding = 1) uniform ubo {
    mat4 cameraRotation;
    uvec2 screenDimensions; 
    float yfovRad;
};

layout (location = 0) in vec2 inUV;

layout (location = 0) out vec4 outFragColor;


const float zNear = 0.1f;
const float zFar = 10.f;

//https://stackoverflow.com/a/62947238/15394869
float GetDistanceFromCamera(float depth) 
{
  float fov = yfovRad; 
  float near = zNear;
  float far = zFar;
  float distance_to_plane = near / (far - depth * (far - near)) * far;


  vec2 center = screenDimensions / 2.0f - 0.5;
  float focal_length = (screenDimensions.y / 2.0f) / tan(fov / 2.0f);
  float diagonal = length(vec3(gl_FragCoord.x - center.x,
                               gl_FragCoord.y - center.y,
                               focal_length));

  return distance_to_plane * (diagonal / focal_length);
}


void main()
{
    float aspect = screenDimensions.x / screenDimensions.y;
   //Convert to NDC
    vec2 uv = gl_FragCoord.xy / screenDimensions.xy;
    uv = uv * 2.0 - 1.0;
    uv.x *= aspect;
    //uv.y *= -1;

    uv *= tan(yfovRad/2);

    

    vec3 ray_direction = normalize(vec3(uv, -1.0));
    ray_direction = (cameraRotation * vec4(ray_direction, 0)).xyz;

    
    //outFragColor = vec4(texture(shadowCubeMap, ray_direction).xxx,1);    
    // texture(shadowCubeMap, ray_direction).r; 
    float sampled = texture(shadowCubeMap, ray_direction).r;
    vec3 mx = mix(sampled.rrr/10, ray_direction, 0.5);
    outFragColor = vec4(sampled.rrr/5,1);
    
    
    //float dist = GetDistanceFromCamera(texture(shadowCubeMap, ray_direction).x)/10000/2;
    //outFragColor = vec4(dist);
}
