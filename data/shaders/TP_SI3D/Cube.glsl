
#version 460

#ifdef VERTEX_SHADER

layout(location= 0) in vec3 position;
layout(location= 1) in vec2 texcoord;
layout(location= 2) in vec3 normal;

// Camera transformations (for all instances)
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

uniform mat4 lightViewMatrix;
uniform mat4 lightProjectionMatrix;

// Per instance data
layout(std430, binding = 0) buffer TransformBlock
{
    mat4 modelMatrix[];
};

out vec3 v_position;
out vec3 v_normal;
out vec2 v_texcoord;
out vec4 v_lightSpacePos;

void main( )
{
    vec4 worldPosition = modelMatrix[gl_InstanceID + gl_BaseInstance] * vec4(position, 1);

    v_position = worldPosition.xyz;
    v_texcoord = texcoord;
    v_normal = mat3(transpose(inverse(modelMatrix[gl_InstanceID + gl_BaseInstance]))) * normal;

    v_lightSpacePos = lightProjectionMatrix * lightViewMatrix * worldPosition;
    
    gl_Position = projectionMatrix * viewMatrix * worldPosition;
}
#endif


#ifdef FRAGMENT_SHADER

in vec2 v_texcoord;
in vec3 v_normal;
in vec3 v_position;
in vec4 v_lightSpacePos;

uniform mat4 viewMatrix;
uniform sampler2D spriteSheet;
uniform sampler2D shadowMap;

out vec4 fragment_color;

float computeShadow()
{
    #if 1
    vec3 projSpacePos = v_lightSpacePos.xyz / v_lightSpacePos.w;
    projSpacePos = projSpacePos / 2.0 + 0.5;
    float minDepth = texture(shadowMap, projSpacePos.xy).x;
    float fragDepth = projSpacePos.z;
    
    return fragDepth > (minDepth + 0.007) ? 1.0 : 0.0;
    #endif

    #if 0

    return 0.0;
    #endif
}

void main( )
{
    vec3 viewPosition = -vec3((viewMatrix) * vec4(0, 0, 0, 1));
    vec3 viewDir = normalize(v_position - viewPosition);
    const vec3 lightDir = -normalize(vec3(2.0, -1.0, 1.0));
    float diffuseLightFactor = max(0.0, dot(lightDir, normalize(v_normal)));
    vec3 lightColor = vec3(1.0, 1.0, 1.0);
    vec3 baseColor = texture(spriteSheet, v_texcoord).xyz;
    float shadow = computeShadow();
    if (shadow == 1.0)
        fragment_color = vec4(0.1 * diffuseLightFactor * lightColor * baseColor, 1.0);
    else
        fragment_color = vec4(diffuseLightFactor * lightColor * baseColor, 1.0);
}
#endif
