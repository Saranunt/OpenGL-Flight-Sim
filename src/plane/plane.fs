#version 330 core
// Lit textured fragment shader with simple PCF-based directional shadows.
out vec4 FragColor;

in VS_OUT
{
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
    vec4 FragPosLightSpace;
} fs_in;

uniform sampler2D texture_diffuse1;
uniform sampler2D shadowMap;

uniform vec3 lightDir;
uniform vec3 viewPos;

float ShadowCalculation(vec4 fragPosLightSpace, vec3 normal, vec3 lightDirection)
{
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    if (projCoords.z > 1.0)
        return 0.0;

    float bias = max(0.0005 * (1.0 - dot(normal, -lightDirection)), 0.0005);
    float currentDepth = projCoords.z;
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for (int x = -1; x <= 1; ++x)
    {
        for (int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
        }
    }
    shadow /= 9.0;
    return shadow;
}

void main()
{
    vec3 color = texture(texture_diffuse1, fs_in.TexCoords).rgb;
    vec3 normal = normalize(fs_in.Normal);
    vec3 lightDirection = normalize(lightDir);
    vec3 viewDirection = normalize(viewPos - fs_in.FragPos);

    float diff = max(dot(normal, -lightDirection), 0.0);
    vec3 diffuse = diff * vec3(1.0);
    vec3 ambient = vec3(0.2);
    vec3 halfwayDir = normalize(-lightDirection + viewDirection);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);
    vec3 specular = vec3(0.2) * spec;

    float shadow = ShadowCalculation(fs_in.FragPosLightSpace, normal, lightDirection);
    vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular)) * color;
    FragColor = vec4(lighting, 1.0);
}