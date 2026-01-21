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

void World::UpdateChunks(Vector3 referencePosition, DeviceResources* deviceRes)
{
	Vector3 refChunkPosition = WorldToChunkPosition(referencePosition);
	for (auto it = chunks.begin(); it != chunks.end();) {
		if (Vector3::Distance(it->second.position, refChunkPosition) > maxChunkDistance) {
			it = chunks.erase(it);
			continue;
		}
		it++;
	}

	bool updated = false;
	for (float x = -chunkGenerationSize.x / 2.0; x < chunkGenerationSize.x / 2.0; x++) {
		for (float y = -chunkGenerationSize.y / 2.0; y < chunkGenerationSize.y / 2.0; y++) {
			for (float z = -chunkGenerationSize.z / 2.0; z < chunkGenerationSize.z / 2.0; z++) {
				Vector3 position = Vector3(floor(x), floor(y), floor(z)) + refChunkPosition;
				if (position.y > 3) continue;
				if (Vector3::Distance(refChunkPosition, position) > minChunkDistance) continue;
				if (chunks.find(position) != chunks.end()) continue;
				chunks[position] = Chunk(position);
				chunks[position].Generate(deviceRes);
				updated = true;
			}
		}
	}

	if (updated) {
		for (auto& it : chunks) {
			it.second.GenerateMesh(deviceRes, this);
		}
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
	Vector3 chunkPosition = WorldToChunkPosition(worldPosition);
	if (chunks.find(chunkPosition) == chunks.end()) return 0;
	return chunks[chunkPosition].GetCubeAtPosition(worldPosition - chunks[chunkPosition].worldPosition);
}

Vector3 World::WorldToChunkPosition(Vector3 worldPosition) {
	return Vector3(
		floor(worldPosition.x / CHUNK_SIZE.x),
		floor(worldPosition.y / CHUNK_SIZE.y),
		floor(worldPosition.z / CHUNK_SIZE.z)
	);
}

Vector3 World::ChunkToWorldPosition(Vector3 chunkPosition) {
	return Vector3(
		chunkPosition.x * CHUNK_SIZE.x,
		chunkPosition.y * CHUNK_SIZE.y,
		chunkPosition.z * CHUNK_SIZE.z
	);
}
