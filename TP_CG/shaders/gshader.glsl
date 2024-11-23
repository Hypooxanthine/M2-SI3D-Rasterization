
#version 440

#ifdef VERTEX_SHADER
uniform mat4 modelMatrix;
uniform mat4 mvpMatrix;
uniform mat4 normalMatrix;

layout(location = 0) in vec3 position;
layout(location = 2) in vec3 normal;

out vec3 v_position;
out vec3 v_normal;

void main( )
{
    v_normal = (normalMatrix * vec4(normal, 1)).xyz;
    v_position = (modelMatrix * vec4(position, 1)).xyz;
    gl_Position = mvpMatrix * vec4(position, 1);
}

#endif

#ifdef FRAGMENT_SHADER

in vec3 v_position;
in vec3 v_normal;

layout(location = 0) out vec4 g_position_matid;
layout(location = 1) out vec4 g_normal_shininess;
layout(location = 2) out vec3 g_albedo;
layout(location = 3) out vec3 g_metallic;
layout(location = 4) out vec3 g_roughness;
layout(location = 5) out vec3 g_specular;

void main( )
{
    g_position_matid.xyz = v_position;
    g_normal_shininess.xyz = v_normal;

    #if 1 // Phong: matid = 0
        g_position_matid.w = 0;
        g_normal_shininess.w = 10.0;
    #endif
}

#endif
