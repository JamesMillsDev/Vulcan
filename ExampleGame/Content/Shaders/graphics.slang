struct MaterialUniform
{
    float4 color;
    float4 emissiveTint;

    float ao;
    float roughness;
    float metallic;
    float alphaMask;
    float alphaMaskCutoff;

    int32_t baseColorIndex;
    int32_t normalIndex;
    int32_t ormIndex;
    int32_t emissiveIndex;
	int32_t heightIndex;
};

[[vk::binding(4, 0)]] ConstantBuffer<MaterialUniform> material;
[[vk::binding(5, 0)]] Sampler2D textures[];

float4 TrySampleMap(int32_t index, float2 uv, float4 defaultValue)
{
    if (NonUniformResourceIndex(index) == -1)
    {
        return defaultValue;
    }

    return textures[NonUniformResourceIndex(index)].Sample(uv) * defaultValue;
}