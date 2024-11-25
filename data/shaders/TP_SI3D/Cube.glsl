
#version 430

#ifdef VERTEX_SHADER

layout(location= 0) in vec3 position;
layout(location= 1) in vec2 texcoord;
layout(location= 2) in vec3 normal;

// Camera transformations (for all instances)
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

// Per instance data
layout(std140) uniform instanceData
{
    mat4 modelMatrix[1];
};

out vec3 v_position;
out vec3 v_normal;
out vec2 v_texcoord;

void main( )
{
    vec4 worldPosition = modelMatrix[gl_InstanceID] * vec4(position, 1);

    v_position = worldPosition.xyz;
    v_texcoord = texcoord;
    v_normal = mat3(transpose(inverse(modelMatrix[gl_InstanceID]))) * normal;
    
    gl_Position = projectionMatrix * viewMatrix * worldPosition;
}
#endif


#ifdef FRAGMENT_SHADER

in vec2 v_texcoord;
in vec3 v_normal;
in vec3 v_position;

uniform mat4 viewMatrix;
layout(location = 0) uniform sampler2D spriteSheet;

out vec4 fragment_color;

void main( )
{
    vec3 viewPosition = -vec3((viewMatrix) * vec4(0, 0, 0, 1));
    vec3 viewDir = normalize(v_position - viewPosition);
    const vec3 lightDir = -normalize(vec3(2.0, -1.0, 1.0));
    float diffuseLightFactor = max(0.0, dot(lightDir, normalize(v_normal)));
    vec3 lightColor = vec3(1.0, 1.0, 1.0);
    vec3 baseColor = texture(spriteSheet, v_texcoord).xyz;
    
    fragment_color = vec4(diffuseLightFactor * lightColor * baseColor, 1.0);
}
#endif
