#pragma once
#include "Engine/VertexLayout.h"
#include "Engine/Buffer.h"

using namespace DirectX::SimpleMath;

class Chunk {
public:
	std::map<Vector3, int> cubes;
	VertexBuffer<VertexLayout_PositionNormalUV> vertexBuffer;
	IndexBuffer indexBuffer;

	Vector3 size = { 32, 64, 32 };

	Chunk() {};
	
	void Generate(DeviceResources* deviceRes);
	void Draw(DeviceResources* deviceRes);
	void PushFace(Vector3 pos, Vector3 up, Vector3 right, Vector2 tilePos);
};