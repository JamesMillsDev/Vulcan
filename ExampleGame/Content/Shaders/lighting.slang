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

const static int MAX_LIGHT_COUNT = 10;
const static float2 INV_ATAN = float2(0.1591, 0.3183);

[[vk::binding(1, 0)]] ConstantBuffer<SceneLightingUniform> sceneLighting;
[[vk::binding(2, 0)]] ConstantBuffer<LightUniform> lights[MAX_LIGHT_COUNT];
[[vk::binding(3, 0)]] Sampler2D skybox;

float2 UvMapSphere(float3 v)
{
    float3 dir = normalize(v);

    float2 uv = float2(atan2(dir.z, dir.x), asin(dir.y));
    uv *= INV_ATAN;
    uv += 0.5;
    return uv;
}

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

float LightAttenuation(int32_t index, float distance)
{
    LightUniform light = lights[index];

    if (light.enabled == 0)
    {
        return 0;
    }

    if (light.type == 0)
    {
        return 1.0 / (distance * distance);
    }

    return 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
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

float3 ImportanceSampleGGX(float2 Xi, float3 N, float roughness)
{
    float a = roughness * roughness;

    float phi = 2.0 * PI * Xi.x;
    float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a * a - 1.0) * Xi.y));
    float sinTheta = sqrt(1.0 - cosTheta * cosTheta);

    // From spherical coords to cartesian coords
    float3 H;
    H.x = cos(phi) * sinTheta;
    H.y = sin(phi) * sinTheta;
    H.z = cosTheta;

    // From tangent-space vector to world-space sample vector
    float3 up = abs(N.z) < 0.999 ? float3(0.0, 0.0, 1.0) : float3(1.0, 0.0, 0.0);
    float3 tangent = normalize(cross(up, N));
    float3 biTangent = cross(N, tangent);

    float3 sampleVec = tangent * H.x + biTangent * H.y + N * H.z;
    return normalize(sampleVec);
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