import global;
import graphics;

[shader("vertex")]
VSOutput main(VSInput input)
{
    VSOutput output = TransferVertex(input);

    output.fragLocation = input.location.xyz;
    output.location = mul(globals.proj, mul(globals.rotationView, input.location)).xyww;

    return output;
}

[shader("fragment")]
float4 main(VSOutput input) : SV_Target
{
    float3 color = skybox.Sample(input.fragLocation.xyz).rgb;

    //color = color / (color + float3(1.0));

    return float4(color, 1.0);
}