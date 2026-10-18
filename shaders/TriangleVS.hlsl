cbuffer TransformConstants : register(b0) {
    float4x4 world;
    float4x4 worldViewProj;
};

struct VSInput {
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 texcoord : TEXCOORD;
    float3 tangent : TANGENT;
};

struct VSOutput {
    float4 position : SV_POSITION;
    float3 worldNormal : NORMAL;
    float2 texcoord : TEXCOORD;
};

VSOutput main(VSInput input) {
    VSOutput output;
    float4 localPosition = float4(input.position, 1.0f);
    output.position = mul(localPosition, worldViewProj);
    output.worldNormal = mul(float4(input.normal, 0.0f), world).xyz;
    output.texcoord = input.texcoord;
    return output;
}
