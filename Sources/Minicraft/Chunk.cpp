#include "pch.h"
#include "Chunk.h"
#include "Perlin.h"
#include "World.h"
#include <thread>
#include "Block.h"

void Chunk::Generate(DeviceResources* deviceRes)
{
	siv::PerlinNoise noise;
	float minElevation = 0.0;
	float maxElevation = CHUNK_SIZE.y;

	for (float x = 0; x < CHUNK_SIZE.x; x++) {
		for (float y = 0; y < CHUNK_SIZE.y; y++) {
			for (float z = 0; z < CHUNK_SIZE.z; z++) {
				Vector3 pos = worldPosition + Vector3(x, y, z);
				double rate = (noise.noise3D_01(pos.x / 10.0, pos.y / 10.0, pos.z / 10.0) + noise.noise3D_01(pos.x / 50.0, pos.y / 50.0, pos.z / 50.0)) / 2.0;
				BlockId cubeValue = EMPTY;

				float elevation = noise.normalizedOctave2D_01(pos.x / 50.0, pos.z / 50.0, 3);
				float stoneElevation = noise.normalizedOctave2D_01((pos.x + 12.5) / 50.0, (pos.z + 21.5) / 50.0, 3);

				if (pos.y < elevation * (maxElevation - minElevation) + minElevation){
					cubeValue = GRASS;
					if (pos.y < elevation * (maxElevation - minElevation) + minElevation - 10.0 * stoneElevation) {
						cubeValue = STONE;
					}
					if (pos.y <= minElevation + maxElevation / 2.0) {
						if (rate < 0.5) {
							cubeValue = EMPTY;
						}
					}
				}
				SetCubeAtPosition(Vector3(x, y, z), cubeValue);
			}
		}
	}

}


void Chunk::GenerateMesh(DeviceResources* deviceRes, World* world) {
	vertexBuffer.Clear();
	indexBuffer.Clear();

	
	for (int index = 0; index < CHUNK_CUBES_COUNT; index++) {
		if (cubes[index] == 0) continue;
		Vector3 pos = GetPositionFromIndex(index);
		PushBlock(pos, world, cubes[index]);
		pos = Vector3::Zero;
	}

	vertexBuffer.Create(deviceRes);
	indexBuffer.Create(deviceRes);
}

Vector3 Chunk::GetPositionFromIndex(int index) {
	return Vector3(
		index / int(CHUNK_SIZE.y * CHUNK_SIZE.z),
		index / int(CHUNK_SIZE.z) % int(CHUNK_SIZE.y),
		index % int(CHUNK_SIZE.z)
	);
}

int Chunk::GetIndexFromPosition(Vector3 pos) {
	return int(pos.x * CHUNK_SIZE.y * CHUNK_SIZE.z + pos.y * CHUNK_SIZE.z + pos.z);
}

BlockId Chunk::GetCubeAtPosition(Vector3 pos) {
	int index = GetIndexFromPosition(pos);
	// TODO: Check neighbouring chunk
	if (index < 0 || index > CHUNK_CUBES_COUNT) return EMPTY;
	if (
		pos.x < 0 || pos.x >= CHUNK_SIZE.x
		|| pos.y < 0 || pos.y >= CHUNK_SIZE.y
		|| pos.z < 0 || pos.z >= CHUNK_SIZE.z
	) return EMPTY;
	return cubes[index];
}

void Chunk::SetCubeAtPosition(Vector3 pos, BlockId cube) {
	int index = GetIndexFromPosition(pos);
	if (index < 0 || index > CHUNK_CUBES_COUNT) {
		return;
	}
	cubes[index] = cube;
}

void Chunk::PushFace(Vector3 pos, Vector3 up, Vector3 right, int texId) {
	Vector2 uv(
		texId % 16,
		texId / 16
	);
	float tileCount = 16;
	//pos -= Vector3(CHUNK_SIZE.x, CHUNK_SIZE.y, CHUNK_SIZE.z) / 2.0;
	Vector3 normal = right.Cross(up);
	int i0 = vertexBuffer.PushVertex(VertexLayout_PositionNormalUV(pos + up, normal, (uv + Vector2(0, 0)) / tileCount));
	int i1 = vertexBuffer.PushVertex(VertexLayout_PositionNormalUV(pos + right, normal, (uv + Vector2(1, 1)) / tileCount));
	int i2 = vertexBuffer.PushVertex(VertexLayout_PositionNormalUV(pos, normal, (uv + Vector2(0, 1)) / tileCount));
	int i3 = vertexBuffer.PushVertex(VertexLayout_PositionNormalUV(pos + up + right, normal, (uv + Vector2(1, 0)) / tileCount));
	indexBuffer.PushTriangle(i0, i1, i2);
	indexBuffer.PushTriangle(i0, i3, i1);
}

void Chunk::PushBlock(Vector3 pos, World* world, BlockId blockId) {
	int sideTexture;
	BlockData bdata = BlockData::Get(blockId);
	sideTexture = world->GetCubeAtPosition(worldPosition + pos + Vector3::Up) != EMPTY ? bdata.texIdBottom
		: bdata.texIdSide;
	if (world->GetCubeAtPosition(worldPosition + pos + Vector3::Left) == EMPTY) {
		PushFace(pos + Vector3::Forward, Vector3::Up, Vector3::Backward, sideTexture);
	}
	if (world->GetCubeAtPosition(worldPosition + pos + Vector3::Right) == EMPTY) {
		PushFace(pos + Vector3::Right, Vector3::Up, Vector3::Forward, sideTexture);
	}
	if (world->GetCubeAtPosition(worldPosition + pos + Vector3::Up) == EMPTY) {
		PushFace(pos + Vector3::Up, Vector3::Forward, Vector3::Right, bdata.texIdTop);
	}
	if (world->GetCubeAtPosition(worldPosition + pos + Vector3::Down) == EMPTY) {
		PushFace(pos + Vector3::Zero, Vector3::Right, Vector3::Forward, bdata.texIdBottom);
	}
	if (world->GetCubeAtPosition(worldPosition + pos + Vector3::Forward) == EMPTY) {
		PushFace(pos + Vector3::Right + Vector3::Forward, Vector3::Up, Vector3::Left, sideTexture);
	}
	if (world->GetCubeAtPosition(worldPosition + pos + Vector3::Backward) == EMPTY) {
		PushFace(pos + Vector3::Zero, Vector3::Up, Vector3::Right, sideTexture);
	}
}


void Chunk::Draw(DeviceResources* deviceRes) {
	if (indexBuffer.Size() == 0) return;
	vertexBuffer.Apply(deviceRes);
	indexBuffer.Apply(deviceRes);
	deviceRes->GetD3DDeviceContext()->DrawIndexed(indexBuffer.Size(), 0, 0);
}
