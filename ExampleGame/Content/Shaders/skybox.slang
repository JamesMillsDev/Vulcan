import global;
import graphics;

struct PushConstants
{
    float4x4 transform;
    MaterialUniform *material;
}

// Bind the push constants layout
[[vk::push_constant]]
PushConstants pushConstants;

[shader("vertex")]
VSOutput main(VSInput input)
{
    VSOutput output = TransferVertex(input, pushConstants.transform);

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