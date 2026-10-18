cbuffer MaterialConstants : register(b1) {
    float4 baseColor;
    float4 emissive;
    float roughness;
    float metallic;
    float specularPower;
    float opacity;
};

cbuffer LightConstants : register(b2) {
    float4 lightDirection;
    float4 lightColor;
    float intensity;
    float3 lightPadding;
};

struct PSInput {
    float4 position : SV_POSITION;
    float3 worldNormal : NORMAL;
    float2 texcoord : TEXCOORD;
};

float4 main(PSInput input) : SV_TARGET {
    float3 normal = normalize(input.worldNormal);
    float3 toLight = normalize(-lightDirection.xyz);
    float ndotl = saturate(dot(normal, toLight));
    float3 ambient = baseColor.rgb * 0.15f;
    float3 diffuse = baseColor.rgb * lightColor.rgb * intensity * ndotl;
    float3 color = ambient + diffuse + emissive.rgb;
    return float4(color, baseColor.a * opacity);
}
