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
    VSOutput output;

    output.location = TransformVertex(input.location, pushConstants.transform);
    output.fragLocation = mul(pushConstants.transform, input.location).xyz;

    output.normal = input.normal.xyz;
    output.tangent = input.tangent;
    
    output.uv0 = input.uv0;
    output.color = input.color;

    return output;
}

[shader("fragment")]
float4 main(VSOutput input) : SV_Target
{
    return pushConstants.material->color;
}