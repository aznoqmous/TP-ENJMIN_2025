struct Input {
    float4 pos : SV_POSITION;
    float4 localPos : VPOS;
};

float4 main(Input input) : SV_TARGET {
    return float4(input.localPos.xyz / 2.0 + float3(0.5, 0.5, 0.5), 1);
}