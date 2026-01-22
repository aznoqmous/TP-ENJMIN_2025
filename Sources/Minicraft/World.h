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
	Vector3 chunkGenerationSize = {2, 2, 2};
	float chunkUnloadDistance = 5; // delete chunks over distance
	float chunkLoadDistance = 3; // create chunks under distance

	void Generate(DeviceResources* deviceRes);
	void Draw(DeviceResources* deviceRes);
	int GetCubeAtPosition(Vector3 worldPosition);
	void UpdateChunks(Vector3 referencePosition, DeviceResources* deviceRes);
	Vector3 WorldToChunkPosition(Vector3 worldPosition);
	Vector3 ChunkToWorldPosition(Vector3 chunkPosition);
	std::list<Chunk*> World::GetNeighbourChunks(Vector3 chunkPosition);
};