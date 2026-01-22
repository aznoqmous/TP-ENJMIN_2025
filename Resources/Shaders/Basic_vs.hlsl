
struct Input {
	float3 pos : POSITION0;
    float2 uv : TEXCOORD0;
    float3 normal : NORMAL0;
};

struct Output {
	float4 pos : SV_POSITION;
    float4 worldPos : WORLD_POS;
    float2 uv : TEXCOORD0;
    float3 normal : NORMAL0;
    float3 selectedCube : SELECTED_CUBE;
};

cbuffer ModelData : register(b0) { // b0 = VSSetConstantBuffers(0,…)
    float4x4 Model;
    float3 SelectedCube;
};

cbuffer CameraData : register(b1) { // b1 = VSSetConstantBuffers(1,…)
    float4x4 View;
    float4x4 Projection;
};

Output main(Input input) {
	Output output = (Output)0;
    
    float4 res = float4(input.pos, 1);
    
    res = mul((float4) float4(input.pos, 1), Model);
    output.worldPos = res;
    
    res = mul(res, View);
    res = mul(res, Projection);
   
    
    output.pos = res;
    output.uv = input.uv;
    output.normal = mul(input.normal, Model);
    output.selectedCube = SelectedCube;
	return output;
}