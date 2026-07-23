float4 Sample(ConstantBuffer<Sampler2D> textures[], int32_t index, float2 uv, float4 defaultValue = float4(0, 0, 0, 0))
{
    if (index == -1)
    {
        return defaultValue;
    }

    return textures[index].Sample(uv);
}