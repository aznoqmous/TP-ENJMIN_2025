#pragma once
#include "Engine/VertexLayout.h"
#include "Engine/Buffer.h"
#include "Block.h"

const Vector3 CHUNK_SIZE = { 16, 32, 16 };
const int CHUNK_CUBES_COUNT = 16 * 32 * 16;
using namespace DirectX::SimpleMath;

class World;
class Chunk {
public:
	Vector3 position;
	Vector3 worldPosition;

	bool isGenerating = false;

	BlockId cubes[CHUNK_CUBES_COUNT];
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
	void PushFace(Vector3 pos, Vector3 up, Vector3 right, int texId);
	void PushBlock(Vector3 pos, World* world, BlockId blockId);

	BlockId GetCubeAtPosition(Vector3 pos);
	void SetCubeAtPosition(Vector3 pos, BlockId cube);
	Vector3 GetPositionFromIndex(int index);
	int GetIndexFromPosition(Vector3 pos);
};