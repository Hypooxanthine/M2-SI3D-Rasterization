
#version 440

#ifdef COMPUTE_SHADER

#define M_PI 3.1415926535897932384626433832795

struct PointLight
{
    vec4 position;
    vec4 color;
    float intensity;
    float radius;
};

layout(std430, binding = 0) buffer LightBlock
{
    uint pointLightCount;
    PointLight pointLights[];
};

layout(local_size_x = 8, local_size_y = 8, local_size_z = 1) in;

uniform sampler2D g_position_matid;
uniform sampler2D g_normal;
uniform sampler2D g_albedo;
uniform sampler2D g_metallic_diffuse_shininess;

uniform int frameWidth;
uniform int frameHeight;
uniform vec3 cameraPos;

layout(binding = 0, rgba32f) writeonly uniform image2D outputTexture;
layout(binding = 1, rgba32f) writeonly uniform image2D debugTexture;

vec3 computePixelColor(ivec2 pixel, vec4 position_matid)
{
        float matid = position_matid.w;
        vec3 p = position_matid.xyz;
    vec3 normal = texelFetch(g_normal, pixel, 0).xyz;
    vec3 albedo = texelFetch(g_albedo, pixel, 0).xyz;
    vec3 metallic_diffuse_shininess = texelFetch(g_metallic_diffuse_shininess, pixel, 0).xyz;
        float metallic = metallic_diffuse_shininess.x;
        float diffuse = metallic_diffuse_shininess.y;
        float shininess = metallic_diffuse_shininess.z;

    vec3 color = vec3(0.0);
    for (uint i = 0; i < pointLightCount; i++)
    {
        PointLight pl = pointLights[i];
        vec3 source = pl.position.xyz;
        vec3 lightColor = pl.color.xyz;
        
        vec3 o = normalize(cameraPos - p);
        vec3 l = normalize(source - p);
        vec3 h = normalize(o + l);
        vec3 nn = normalize(normal);

        float cos_theta = max(dot(nn, l), 0.0);
        float cos_theta_h = dot(nn, h);

        // Composante diffuse
        float diffuseFactor = diffuse / M_PI;
        vec3 diffuseColor = diffuseFactor * lightColor * albedo * cos_theta;

        // Composante spéculaire
        float specularFactor = (shininess + 8) / (M_PI * 8) * pow(cos_theta_h, shininess);
        vec3 specularColor = specularFactor * lightColor * cos_theta;

        color += ((1.0 - metallic) * diffuseColor + specularColor);
    }

    vec3 toto = vec3(1.0, 1.0, 1.0);

    // for (int i = 0; i < 100000; ++i)
    // {
    //     toto = sqrt(toto);
    // }

    // return max(vec3(0., 0., 0.), color * toto / toto);
    return max(vec3(0., 0., 0.), color);
}

void main( )
{
    ivec2 tile = ivec2(gl_GlobalInvocationID.xy);
    ivec2 px = tile * ivec2(4, 4);
    vec4 position_matid = texelFetch(g_position_matid, px, 0);
    if (position_matid.w == 0.0) return;

    vec3 color = computePixelColor(px, position_matid);

    imageStore(outputTexture, px, vec4(color, 1.0));
    imageStore(debugTexture, px, vec4(1, 0, 0, 1.0));
}

#endif
