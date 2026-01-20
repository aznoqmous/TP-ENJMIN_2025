Texture2D tex : register(t0);
SamplerState sampleState : register(s0);

struct Input {
    float4 pos : SV_POSITION;
    float4 localPos : VPOS;
    float2 uv : TEXCOORD0;
};

float4 main(Input input) : SV_TARGET {
    //return float4(input.uv, 0, 1);
    
    float4 color = tex.Sample(sampleState, input.uv);
    
    return color;
}