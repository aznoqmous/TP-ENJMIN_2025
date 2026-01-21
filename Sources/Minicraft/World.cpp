#include "pch.h"
#include "World.h"

void World::Generate(DeviceResources* deviceRes) {
	modelBuffer.Create(deviceRes);
	for (float x = 0; x < chunkGenerationSize.x; x++) {
		for (float y = 0; y < chunkGenerationSize.y; y++) {
			for (float z = 0; z < chunkGenerationSize.z; z++) {
				Vector3 position = Vector3(x, y, z);
				chunks[position] = Chunk(position);
				chunks[position].Generate(deviceRes);
			}
		}
	}

	
	for (auto &it : chunks) {
		it.second.GenerateMesh(deviceRes, this);
	}
}

void World::Draw(DeviceResources* deviceRes) {
	for (auto &it: chunks) {
		modelBuffer.data.modelMatrix = Matrix::CreateTranslation(it.second.worldPosition).Transpose();
		modelBuffer.UpdateBuffer(deviceRes);
		modelBuffer.ApplyToVS(deviceRes, 0);
		it.second.Draw(deviceRes);
	}
}

int World::GetCubeAtPosition(Vector3 worldPosition) {
	Vector3 chunkPosition = Vector3(
		floor(worldPosition.x / CHUNK_SIZE.x),
		floor(worldPosition.y / CHUNK_SIZE.y),
		floor(worldPosition.z / CHUNK_SIZE.z)
	);
	if (chunks.find(chunkPosition) == chunks.end()) return 0;
	return chunks[chunkPosition].GetCubeAtPosition(worldPosition - chunks[chunkPosition].worldPosition);
}