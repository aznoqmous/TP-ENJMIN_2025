#include "pch.h"
#include "World.h"
#include <thread>
#include <list>
#include <algorithm>

void World::GenerateWorker(DeviceResources* deviceRes, Chunk* chunk) {
	chunkGenJob.acquire();
	chunk->GenerateMesh(deviceRes, this);
	chunk->isGenerating = false;
	chunkGenJob.release();
}


void World::Generate(DeviceResources* deviceRes) {
	modelBuffer.Create(deviceRes);
	chunks.clear();
	staleChunks.clear();
	/*
	for (float x = 0; x < chunkGenerationSize.x; x++) {
		for (float y = 0; y < chunkGenerationSize.y; y++) {
			for (float z = 0; z < chunkGenerationSize.z; z++) {
				Vector3 position = Vector3(x, y, z);
				chunks[position] = Chunk(position);
				chunks[position].Generate(deviceRes);
				staleChunks.push_back(&chunks[position]);
			}
		}
	}
	*/
}

void World::UpdateChunks(Vector3 referencePosition, DeviceResources* deviceRes)
{
	Vector3 refChunkPosition = WorldToChunkPosition(referencePosition);
	for (auto it = chunks.begin(); it != chunks.end();) {
		if (Vector3::Distance(it->second.position, refChunkPosition) > chunkUnloadDistance && !it->second.isGenerating) {
			it = chunks.erase(it);
			continue;
		}
		it++;
	}

	std::map<Vector3, Chunk*> needUpdateChunks;
	for (float x = -chunkGenerationSize.x / 2.0; x < chunkGenerationSize.x / 2.0; x++) {
		for (float y = -chunkGenerationSize.y / 2.0; y < chunkGenerationSize.y / 2.0; y++) {
			for (float z = -chunkGenerationSize.z / 2.0; z < chunkGenerationSize.z / 2.0; z++) {
				Vector3 position = Vector3(floor(x), floor(y), floor(z)) + refChunkPosition;
				if (position.y < 0) continue;
				if (Vector3::Distance(refChunkPosition, position + Vector3::One / 2.0) > chunkLoadDistance) continue;
				if (chunks.find(position) != chunks.end()) continue;
				chunks[position] = Chunk(position);
				chunks[position].Generate(deviceRes);
				needUpdateChunks[position] = &chunks[position];
				for (Chunk* nchunk : GetNeighbourChunks(position)) {
					if (needUpdateChunks.find(position) != needUpdateChunks.end()) continue;
					if (std::find(staleChunks.begin(), staleChunks.end(), &chunks[position]) != staleChunks.end()) continue;
					needUpdateChunks[position] = nchunk;
				}
			}
		}
	}
	
	for (auto& it : needUpdateChunks) {
		staleChunks.push_back(it.second);
	}

	std::vector<Chunk*>::iterator it = staleChunks.begin();
	while (it != staleChunks.end()) {
		if ((*it)->isGenerating) {
			++it;
			continue;
		}
		(*it)->isGenerating = true;
		std::thread task(&World::GenerateWorker, this, deviceRes, *it);
		it = staleChunks.erase(it);
		task.detach();
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

BlockId World::GetCubeAtPosition(Vector3 worldPosition) {
	Vector3 chunkPosition = WorldToChunkPosition(worldPosition);
	if (chunks.find(chunkPosition) == chunks.end()) return EMPTY;
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

std::list<Chunk*> World::GetNeighbourChunks(Vector3 chunkPosition) {
	std::list<Chunk*> nchunks;
	Vector3 positions[6] = {
		chunkPosition + Vector3::Up,
		chunkPosition + Vector3::Down,
		chunkPosition + Vector3::Left,
		chunkPosition + Vector3::Right,
		chunkPosition + Vector3::Forward,
		chunkPosition + Vector3::Backward,
	};
	for (Vector3 pos : positions) {
		if (chunks.find(pos) == chunks.end()) continue;
		nchunks.push_back(&chunks[pos]);
	}
	return nchunks;
}

void World::ShowImGui(DeviceResources* res) {
	ImGui::Begin("World gen");

	//ImGui::DragFloat("perlinScaleStone", &perlinScaleStone, 0.01f);
	//ImGui::DragInt("perlinOctaveStone", &perlinOctaveStone, 0.1f);
	//ImGui::DragFloat("perlinHeightStone", &perlinHeightStone, 0.1f);
	//ImGui::DragFloat("perlinScaleDirt", &perlinScaleDirt, 0.01f);
	//ImGui::DragInt("perlinOctaveDirt", &perlinOctaveDirt, 0.1f);
	//ImGui::DragFloat("perlinHeightDirt", &perlinHeightDirt, 0.1f);

	
	if (ImGui::Button("Generate!"))
		Generate(res);
	

	ImGui::End();
}