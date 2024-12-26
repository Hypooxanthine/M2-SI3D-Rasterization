
#version 460

#ifdef VERTEX_SHADER

layout(location= 0) in vec3 position;
layout(location= 1) in vec2 texcoord;
layout(location= 2) in vec3 normal;

// Camera transformations (for all instances)
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

// Per instance data
layout(std430, binding = 0) buffer TransformBlock
{
    mat4 modelMatrix[];
};

void main( )
{
    vec4 worldPosition = modelMatrix[gl_InstanceID + gl_BaseInstance] * vec4(position, 1);
    
    gl_Position = projectionMatrix * viewMatrix * worldPosition;
}
#endif


#ifdef FRAGMENT_SHADER

out vec4 fragment_color;

void main( )
{
    fragment_color = vec4(0.f, 0.f, 0.f, 1.0);
}
#endif
