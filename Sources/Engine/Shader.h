#pragma once

using namespace DirectX::SimpleMath;
using Microsoft::WRL::ComPtr;

class Shader {
	std::wstring shaderName;
	std::vector<uint8_t> psBytecode;

	ComPtr<ID3D11VertexShader>	vertexShader;
	ComPtr<ID3D11PixelShader>	pixelShader;
public:
	std::vector<uint8_t> vsBytecode;
	Shader(std::wstring name) : shaderName(name) {};

	void Create(DeviceResources* deviceRes);
	void Apply(DeviceResources* deviceRes);
};

extern std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D11InputLayout>> g_inputLayouts;

template <typename T>
void GenerateInputLayout(DeviceResources* deviceRes, Shader* basicShader) {
	auto it = g_inputLayouts.find(typeid(T).name());
	if (it != g_inputLayouts.end()) return;
	deviceRes->GetD3DDevice()->CreateInputLayout(
		T::InputElementDescs.data(),
		T::InputElementDescs.size(),
		basicShader->vsBytecode.data(),
		basicShader->vsBytecode.size(),
		g_inputLayouts[typeid(T).name()].ReleaseAndGetAddressOf());
}

template <typename T>
void ApplyInputLayout(DeviceResources* deviceRes) {
	auto it = g_inputLayouts.find(typeid(T).name());
	assert(it != g_inputLayouts.end());
	deviceRes->GetD3DDeviceContext()->IASetInputLayout(it->second.Get());
}