
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
layout(location = 1) out vec4 g_normal;
layout(location = 2) out vec4 g_albedo;
layout(location = 3) out vec4 g_metallic_diffuse_shininess;

void main( )
{
    g_position_matid.xyz = v_position;
    g_normal.xyz = v_normal;

    uint matid;
    vec3 albedo;
    float metallic;
    float diffuse;
    float shininess;

    #if 1 // Blinn-Phong
        matid = 1;
        albedo = vec3(1.0, 0.0, 0.0);
        metallic = .0;
        diffuse = 0.7;
        shininess = 5.0;
    #endif

    g_position_matid.w = float(matid);
    g_albedo = vec4(albedo, 1.0);
    g_metallic_diffuse_shininess = vec4(metallic, diffuse, shininess, 1.0);
}

#endif
