import global;
import graphics;

[shader("vertex")]
VSOutput main(VSInput input)
{
    VSOutput output = TransferVertex(input);

    // Construct a rotation only matrix
    float4x4 rotView = float4x4(1.0);

    rotView[0] = float4(globals.view[0].xyz, 0.0);
    rotView[1] = float4(globals.view[1].xyz, 0.0);
    rotView[2] = float4(globals.view[2].xyz, 0.0);

    float4 clipPos = mul(globals.proj, mul(rotView, input.location));

    output.location = clipPos.xyzw;

    return output;
}

[shader("fragment")]
float4 main(VSOutput input) : SV_Target
{
    float3 normalizedDir = normalize(input.location.xyz);
    float2 uv = SampleSphericalMap(normalizedDir);
    float3 color = TrySampleMap(material.baseColorIndex, uv, float4(1.0)).rgb;

    color = color / (color + float3(1.0));

    return float4(color, 1.0);
}