#pragma once
#include "Engine/VertexLayout.h"
#include "Engine/Buffer.h"

using namespace DirectX::SimpleMath;


class Cube {
private:
	Vector3 position;
	VertexBuffer<VertexLayout_PositionUV> vertexBuffer;
	IndexBuffer indexBuffer;
	Matrix modelMatrix;
	void PushFace(Vector3 pos, Vector3 up, Vector3 right, Vector2 tilePos);

public:
	Cube(Vector3 position) : position(position) {};
	void Generate(DeviceResources* deviceRes);
	Matrix GetModelMatrix();
	void Draw(DeviceResources* deviceRes);
};