#pragma once

#include "Chunk.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;
struct ChunkData {
	Matrix modelMatrix;
};
class World {
public:

	ConstantBuffer<ChunkData> modelBuffer;
	std::map<Vector3, Chunk> chunks;
	Vector3 chunkGenerationSize = {3, 3, 3};
	float maxChunkDistance = 4; // delete chunks over distance
	float minChunkDistance = 3; // create chunks under distance

	void Generate(DeviceResources* deviceRes);
	void Draw(DeviceResources* deviceRes);
	int GetCubeAtPosition(Vector3 worldPosition);
	void UpdateChunks(Vector3 referencePosition, DeviceResources* deviceRes);
	Vector3 WorldToChunkPosition(Vector3 worldPosition);
	Vector3 ChunkToWorldPosition(Vector3 chunkPosition);
};