
#version 440

#ifdef COMPUTE_SHADER

struct PointLight
{
    float position[3];
    float color[3];
    float intensity;
    float radius;
};

layout(std430, binding = 0) buffer LightBlock
{
    uint pointLightCount;
    PointLight pointLights[];
};

layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) in;

layout(location = 0) uniform sampler2D g_position_matid;
layout(location = 1) uniform sampler2D g_normal_shininess;
layout(location = 2) uniform sampler2D g_ambient;
layout(location = 3) uniform sampler2D g_diffuse;
layout(location = 4) uniform sampler2D g_specular;

uniform int frameWidth;
uniform int frameHeight;

layout(binding = 0, rgba32f) uniform image2D outputTexture;

void main( )
{
    ivec2 pixel = ivec2(gl_GlobalInvocationID.xy);
    if (pixel.x >= frameWidth || pixel.y >= frameHeight)
    {
        return;
    }

    vec4 position_matid = texelFetch(g_position_matid, pixel, 0);
        float matid = position_matid.w;
        if (matid == 0.0) return;
        vec3 position = position_matid.xyz;
    vec4 normal_shininess = texelFetch(g_normal_shininess, pixel, 0);
        vec3 normal = normal_shininess.xyz;
        float shininess = normal_shininess.w;
    vec3 ambient = texelFetch(g_ambient, pixel, 0).xyz;
    vec3 diffuse = texelFetch(g_diffuse, pixel, 0).xyz;
    vec3 specular = texelFetch(g_specular, pixel, 0).xyz;

    vec3 color = vec3(0);
    for (uint i = 0; i < pointLightCount; i++)
    {
        vec3 lightPos = vec3(pointLights[i].position[0], pointLights[i].position[1], pointLights[i].position[2]);
        vec3 lightColor = vec3(pointLights[i].color[0], pointLights[i].color[1], pointLights[i].color[2]);

        vec3 lightDir = lightPos - position;
        float distance = length(lightDir);
        lightDir = normalize(lightDir);

        float attenuation = 1.0 / (1.0 + 0.1 * distance + 0.01 * distance * distance);
        float diffuseFactor = max(dot(normal, lightDir), 0.0);
        vec3 diffuseColor = diffuse * lightColor * pointLights[i].intensity * diffuseFactor * attenuation;

        vec3 viewDir = normalize(-position);
        vec3 reflectDir = reflect(-lightDir, normal);
        float specularFactor = pow(max(dot(viewDir, reflectDir), 0.0), 10.0);
        vec3 specularColor = specular * lightColor * pointLights[i].intensity * specularFactor * attenuation;

        color += (diffuseColor + specularColor);
    }

    imageStore(outputTexture, pixel, vec4(color, 1.0));
}

#endif
