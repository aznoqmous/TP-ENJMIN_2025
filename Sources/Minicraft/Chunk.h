#pragma once
#include "Engine/VertexLayout.h"
#include "Engine/Buffer.h"

const Vector3 CHUNK_SIZE = { 8, 8, 8 };
const int CHUNK_CUBES_COUNT = 8 * 8 * 8;
using namespace DirectX::SimpleMath;


class World;
class Chunk {
public:
	//std::map<Vector3, int> cubes;
	Vector3 position;
	Vector3 worldPosition;

	int cubes[CHUNK_CUBES_COUNT];
	VertexBuffer<VertexLayout_PositionNormalUV> vertexBuffer;
	IndexBuffer indexBuffer;


	Chunk() {};
	Chunk(Vector3 position) {
		this->position = position;
		worldPosition = Vector3(position.x * CHUNK_SIZE.x, position.y * CHUNK_SIZE.y, position.z * CHUNK_SIZE.z);
	};
	
	void Generate(DeviceResources* deviceRes);
	void GenerateMesh(DeviceResources* deviceRes, World* world);
	void Draw(DeviceResources* deviceRes);
	void PushFace(Vector3 pos, Vector3 up, Vector3 right, Vector2 tilePos);

	int GetCubeAtPosition(Vector3 pos);
	void SetCubeAtPosition(Vector3 pos, int cube);
	Vector3 GetPositionFromIndex(int index);
	int GetIndexFromPosition(Vector3 pos);
};