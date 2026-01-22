#pragma once

#include "Chunk.h"
#include <semaphore>

using namespace DirectX;
using namespace DirectX::SimpleMath;
struct ChunkData {
	Matrix modelMatrix;
};

const int CONCURRENT_CHUNK_GEN = 3;

class World {
public:
	std::counting_semaphore<CONCURRENT_CHUNK_GEN> chunkGenJob { CONCURRENT_CHUNK_GEN };

	ConstantBuffer<ChunkData> modelBuffer;
	std::map<Vector3, Chunk> chunks;
	std::vector<Chunk*> staleChunks;
	Vector3 chunkGenerationSize = {10, 1, 10};
	float chunkUnloadDistance = 20; // delete chunks over distance
	float chunkLoadDistance = 15; // create chunks under distance

	void GenerateWorker(DeviceResources* deviceRes, Chunk* chunk);
	void Generate(DeviceResources* deviceRes);
	void Draw(DeviceResources* deviceRes);
	int GetCubeAtPosition(Vector3 worldPosition);
	void UpdateChunks(Vector3 referencePosition, DeviceResources* deviceRes);
	Vector3 WorldToChunkPosition(Vector3 worldPosition);
	Vector3 ChunkToWorldPosition(Vector3 chunkPosition);
	std::list<Chunk*> GetNeighbourChunks(Vector3 chunkPosition);
	
};