import global;
import graphics;

[shader("vertex")]
VSOutput main(VSInput input)
{
    VSOutput output = TransferVertex(input);
    
    float4 clipPos = mul(globals.proj, mul(globals.rotationView, input.location));

    output.location = clipPos.xyww;

    return output;
}

[shader("fragment")]
float4 main(VSOutput input) : SV_Target
{
    float3 normalizedDir = normalize(input.fragLocation.xyz);
    float2 uv = SampleSphericalMap(normalizedDir);
    float3 color = TrySampleMap(material.baseColorIndex, uv, float4(1.0)).rgb;

    color = color / (color + float3(1.0));

    return float4(color, 1.0);
}