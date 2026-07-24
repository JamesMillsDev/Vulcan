import global;

struct LightUniform
{
	float4 location;
	float4 direction;
	float4 color;

    float intensity;
    float constant;
    float linear;
    float quadratic;
    float cutOff;
    float outerCutOff;
    
    int32_t type;
    int32_t enabled;
};

struct SceneLightingUniform
{
    float4 ambientColor;
    float ambientStrength;
};

const static int MAX_LIGHT_COUNT = 16;
[[vk::binding(3, 0)]] ConstantBuffer<SceneLightingUniform> sceneLighting;
[[vk::binding(4, 0)]] ConstantBuffer<LightUniform> lights[MAX_LIGHT_COUNT];

float3 LightDirection(int32_t index, float3 location)
{
    // The light is disabled, so return no direction
    if (lights[index].enabled == 0)
    {
        return float3(0, 1, 0);
    }

    // Directional light
    if (lights[index].type == 0)
    {
        return normalize(lights[index].direction.xyz);
	}

    return normalize(lights[index].location.xyz - location);
}

// Normal Distribution Function (D) - GGX/Trowbridge-Reitz Distribution
// Describes the statistical distribution of microfacet orientations
float DistributionGGX(float NdotH, float roughness)
{
    // Remapping for more perceptual linearity
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH2 = NdotH * NdotH;

    // Numerator: concentration factor
    float nom = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    // Normalization factor
    denom = PI * denom * denom;

    // Normalized distribution
    return nom / denom;
}

// Geometry Function (G) - Smith's method with Schlick-GGX approximation
// Models self-shadowing and masking between microfacets
float GeometrySmith(float NdotV, float NdotL, float roughness) 
{
    float r = roughness + 1.0;
    // Direct lighting remapping
    float k = (r * r) / 8.0;

    // Geometry obstruction from view direction (masking)
    float ggx1 = NdotV / (NdotV * (1.0 - k) + k);
    // Geometry obstruction from light direction (shadowing)
    float ggx2 = NdotL / (NdotL * (1.0 - k) + k);

     // Combined masking-shadowing
    return ggx1 * ggx2;
}

// Fresnel Reflectance (F) - Schlick's approximation
// Models how reflectance changes with viewing angle
float3 FresnelSchlick(float cosTheta, float3 F0) 
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}