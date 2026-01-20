
struct Input {
	float3 pos : POSITION0;
    float2 uv : TEXCOORD0;
};

struct Output {
	float4 pos : SV_POSITION;
    float4 localPos : VPOS;
    float2 uv : TEXCOORD0;
};

cbuffer ModelData : register(b0) { // b0 = VSSetConstantBuffers(0,…)
    float4x4 Model;
};

cbuffer CameraData : register(b1) { // b1 = VSSetConstantBuffers(1,…)
    float4x4 View;
    float4x4 Projection;
};

Output main(Input input) {
	Output output = (Output)0;
    
    
    float4 res = float4(input.pos, 1);
    
    res = mul((float4) float4(input.pos, 1), Model);
    res = mul(res, View);
    res = mul(res, Projection);
    
    output.pos = res;
    output.localPos = float4(input.pos, 1);
    output.uv = input.uv;
	return output;
}