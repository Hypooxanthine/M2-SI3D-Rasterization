
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

layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) in;

uniform sampler2D g_position_matid;
uniform sampler2D g_normal;
uniform sampler2D g_albedo;
uniform sampler2D g_metallic_diffuse_shininess;

uniform int frameWidth;
uniform int frameHeight;
uniform vec3 cameraPos;

// Pour savoir quels sont les pixels dont on cherche la couleur
uniform int fillMask;

// Pour chaque pixel dont on cherche la couleur, ces variables permettront
// de savoir quels sont les 4 pixels à utiliser pour interpoler la couleur
// (s'il est décidé d'interpoler)
uniform ivec2 dA;
uniform ivec2 dB;
uniform ivec2 dC;
uniform ivec2 dD;

uniform float varianceThreshold;

layout(binding = 0, rgba32f) uniform image2D outputTexture;
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

    return color;
}

vec3 interpolatePixelColor(ivec2 pixel)
{
    vec3 a = imageLoad(outputTexture, pixel + dA).xyz;
    vec3 b = imageLoad(outputTexture, pixel + dB).xyz;
    vec3 c = imageLoad(outputTexture, pixel + dC).xyz;
    vec3 d = imageLoad(outputTexture, pixel + dD).xyz;

    return (a + b + c + d) / 4.0;
}

float gray(vec3 color)
{
    return 0.21 * color.r + 0.71 * color.g + 0.08 * color.b;
}

float variance(vec3 a, vec3 b, vec3 c, vec3 d)
{
    vec3 s = (a + b + c + d) / 4.0;
    vec3 ss = (a * a + b * b + c * c + d * d) / 4.0;
    return abs(gray(ss - s * s));
}

bool shouldComputePixel(ivec2 px)
{
    // return true;
    // return false;
    
    if (texelFetch(g_position_matid, px + dA, 0).w == 0.0) return true;
    if (texelFetch(g_position_matid, px + dB, 0).w == 0.0) return true;
    if (texelFetch(g_position_matid, px + dC, 0).w == 0.0) return true;
    if (texelFetch(g_position_matid, px + dD, 0).w == 0.0) return true;

    vec3 a = imageLoad(outputTexture, px + dA).xyz;
    vec3 b = imageLoad(outputTexture, px + dB).xyz;
    vec3 c = imageLoad(outputTexture, px + dC).xyz;
    vec3 d = imageLoad(outputTexture, px + dD).xyz;
    return variance(a, b, c, d) > varianceThreshold;
}

void main( )
{
    // pixel = ivec2(gl_GlobalInvocationID.xy);
    // if (pixel.x >= frameWidth || pixel.y >= frameHeight)
    // {
    //     return;
    // }

    vec3 black = vec3(0.0);
    vec3 white = vec3(1.0);

    ivec2 tile = ivec2(gl_GlobalInvocationID.xy);

    for (int i = 0; i < 4 * 4; i++)
    {
        if ((fillMask & (1 << i)) > 0)
        {
            ivec2 px = tile * ivec2(4, 4) + ivec2(i % 4, i / 4);
            vec4 position_matid = texelFetch(g_position_matid, px, 0);
            if (position_matid.w == 0.0) continue;

            vec3 color;

            if (shouldComputePixel(px))
            {
                color = computePixelColor(px, position_matid);
                imageStore(debugTexture, px, vec4(1, 0, 0, 1.0));
            }
            else
            {
                color = interpolatePixelColor(px);
                imageStore(debugTexture, px, vec4(0.3, 0.2, 1, 1.0));
            }

            // color = black;
            // color = computePixelColor(px, position_matid);

            imageStore(outputTexture, px, vec4(color, 1.0));
        }
    }
}

#endif
