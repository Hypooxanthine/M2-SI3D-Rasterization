
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

layout(binding = 0, rgba32f) uniform image2D outputTexture;

ivec2 pixel;
vec4 position_matid;
    float matid;
    vec3 p;
vec3 normal;
vec3 albedo;
vec3 metallic_diffuse_shininess;
    float metallic;
    float diffuse;
    float shininess;

vec3 computePixelColor()
{
        p = position_matid.xyz;
    normal = texelFetch(g_normal, pixel, 0).xyz;
    albedo = texelFetch(g_albedo, pixel, 0).xyz;
    metallic_diffuse_shininess = texelFetch(g_metallic_diffuse_shininess, pixel, 0).xyz;
        metallic = metallic_diffuse_shininess.x;
        diffuse = metallic_diffuse_shininess.y;
        shininess = metallic_diffuse_shininess.z;

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

vec3 interpolatePixelColor()
{
    return vec3(0.f, 0.f, 0.f);
}

bool isPixelComputed()
{
    return gl_LocalInvocationID.x == 0 && gl_LocalInvocationID.y == 0;
}

float gray(vec3 color)
{
    return 0.21f * color.r + 0.71f * color.g + 0.08f * color.b;
}

bool shouldCompute(float threshold)
{
    float neighborsCount = 4.f;
    vec3 s = vec3(0.f);
    vec3 ss = vec3(0.f);

    uint tileWidth = 4;
    uint tileHeight = 4;

    uint xPrev = (gl_GlobalInvocationID.x - gl_GlobalInvocationID.x % tileWidth);
    uint xNext = xPrev + tileWidth;
    bool xNextValid = xNext < frameWidth;

    uint yPrev = (gl_GlobalInvocationID.y - gl_GlobalInvocationID.y % tileHeight);
    uint yNext = yPrev + tileHeight;
    bool yNextValid = yNext < frameHeight;

    vec3 a = imageLoad(outputTexture, ivec2(xPrev, yPrev)).xyz;
    s = s + a;
    ss = ss + a*a;

    if (xNextValid)
    {
        vec3 b = imageLoad(outputTexture, ivec2(xNext, yPrev)).xyz;
        s = s + b;
        ss = ss + b*b;
    }
    else
        neighborsCount = neighborsCount / 2.f;

    if (yNextValid)
    {
        vec3 c = imageLoad(outputTexture, ivec2(xPrev, yNext)).xyz;
        s = s + c;
        ss = ss + c*c;

        if (xNextValid)
        {
            vec3 d = imageLoad(outputTexture, ivec2(xNext, yNext)).xyz;
            s = s + d;
            ss = ss + d*d;
        }
    }
    else
        neighborsCount = neighborsCount / 2.f;

    s = s / neighborsCount;
    ss = ss / neighborsCount;
    float v = gray(ss - s*s);

    return v > threshold;
}

void main( )
{
    pixel = ivec2(gl_GlobalInvocationID.xy);
    if (pixel.x >= frameWidth || pixel.y >= frameHeight)
    {
        return;
    }

    if (isPixelComputed())
        return;

    position_matid = texelFetch(g_position_matid, pixel, 0);
        matid = position_matid.w;
        if (matid == 0.0) return;

    vec3 color;

    if (shouldCompute(0.f))
        color = computePixelColor();
    else
        color = interpolatePixelColor();

    imageStore(outputTexture, pixel, vec4(color, 1.0));
}

#endif
