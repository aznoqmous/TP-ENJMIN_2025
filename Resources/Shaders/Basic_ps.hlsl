Texture2D tex : register(t0);
SamplerState sampleState : register(s0);

struct Input {
    float4 pos : SV_POSITION;
    float4 worldPos : WORLD_POS;
    float2 uv : TEXCOORD0;
    float3 normal : NORMAL0;
    float3 selectedCube : SELECTED_CUBE;
};

float4 main(Input input) : SV_TARGET {
        
    float3 lightDirection = float3(1, -1, -1);
    float lightIntensity = max(dot(normalize(input.normal), normalize(-lightDirection)), 0.1);
    
    float4 color = tex.Sample(sampleState, input.uv);
    
    if (
        abs(input.worldPos.x - input.selectedCube.x) < 1
        && abs(input.worldPos.z - input.selectedCube.z) < 1
    )
    {
        color = float4(1, 1, 1, 1);
    }
    
    return color * lightIntensity;
}