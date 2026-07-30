struct VSInput
{
    float4 location;
    float4 normal;
    float4 tangent;

    float2 uv0;
    float4 color;
};

struct VSOutput
{
    float4 location : SV_Position;
    float3 fragLocation;
    float3 normal;
    float4 tangent;
    
    float2 uv0;
    float4 color;
};

struct GlobalsUniform
{
    float4x4 proj;
    float4x4 view;
    float4x4 rotationView;
    float3 cameraLocation;

    float exposure;
    float gamma;
    float prefilteredCubeMipLevels;
    float scaleIBLAmbient;
};

struct TransformUniform
{
    float4x4 value;
};

static const float PI = 3.14159265359f;

[[vk::binding(0, 0)]] ConstantBuffer<GlobalsUniform> globals;
[[vk::binding(3, 0)]] ConstantBuffer<TransformUniform> transform;

float4 TransformVertex(VSInput vertex)
{
    return mul(globals.proj, mul(globals.view, mul(transform.value, vertex.location)));
}

VSOutput TransferVertex(VSInput vertex)
{
    VSOutput output;

    output.location = TransformVertex(vertex);
    output.fragLocation = output.location.xyz;

    output.normal = vertex.normal.xyz;
    output.tangent = vertex.tangent;

    output.uv0 = vertex.uv0;
    output.color = vertex.color;

    return output;
}

float3 CalculateNormal(VSOutput vertex, float4 normalValue, int32_t normalMapIndex)
{
    // Sample normal map and convert from [0,1] to [-1,1] range
    float3 tangentNormal = normalValue.xyz * 2.0 - 1.0;

    // Construct tangent-space to world-space transformation matrix (TBN)
    float3 T = normalize(vertex.tangent.xyz);                         // Tangent
    float3 B = normalize(cross(vertex.normal, T)) * vertex.tangent.w; // Bitangent (w = handedness)
    float3x3 TBN = float3x3(T, B, vertex.normal);                     // Tangent-Bitangent-Normal matrix

    return normalize(mul(tangentNormal, TBN));
}

float2 SampleSphericalMap(float3 v)
{
    float longitude = atan2(v.z, v.x);
    float latitude = asin(clamp(v.y, -1.0, 1.0));

    float2 uv = float2(longitude / (2.0 * PI) + 0.5, latitude / PI + 0.5);
    return uv;
}