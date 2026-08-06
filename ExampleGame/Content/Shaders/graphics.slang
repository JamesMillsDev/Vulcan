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

[[vk::binding(4, 0)]] Sampler2D textures[];

float4 TrySampleMap(int32_t index, float2 uv, float4 defaultValue)
{
    if (NonUniformResourceIndex(index) == -1)
    {
        return defaultValue;
    }

    return textures[NonUniformResourceIndex(index)].Sample(uv);
}

float3 TryMapNormals(int32_t index, float2 uv, float3 normal, float4 tangent)
{
    // Return the input normal if normal mapping unavailable
    if (NonUniformResourceIndex(index) == -1)
    {
        return normal;
    }

    float3 N = normalize(normal);

    // Sample normal map
    float3 tangentNormal = textures[NonUniformResourceIndex(index)].Sample(uv).xyz * 2.0 - 1.0;

    // Construct the TBN
    float3 T = normalize(tangent.xyz);
    float3 B = normalize(cross(N, T)) * tangent.w;
    float3x3 TBN = float3x3(T, B, N);

    // Transform normal from tangent space to world space
    return normalize(mul(tangentNormal, TBN));
}