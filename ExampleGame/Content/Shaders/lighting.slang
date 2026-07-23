struct LightUniform
{
	float4 location;
	float4 direction;
	float4 color;
	int32_t type;
};

struct SceneLightingUniform
{
    float4 ambientColor;
    float ambientStrength;
};

const static int MAX_LIGHT_COUNT = 8;
[[vk::binding(0, 0)]] ConstantBuffer<SceneLightingUniform> sceneLighting;
[[vk::binding(1, 0)]] ConstantBuffer<LightUniform> lights[MAX_LIGHT_COUNT];

float3 LightDirection(LightUniform light, float3 location)
{
    if (light.type == 0)
    {
        return light.direction.xyz;
	}

    return normalize(light.location.xyz - location);
}