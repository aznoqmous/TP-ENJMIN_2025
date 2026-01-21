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
	Vector3 chunkGenerationSize = {6, 1, 6};

	void Generate(DeviceResources* deviceRes);
	void Draw(DeviceResources* deviceRes);
	int GetCubeAtPosition(Vector3 worldPosition);
};