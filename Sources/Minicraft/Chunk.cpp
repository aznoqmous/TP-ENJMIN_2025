#include "pch.h"
#include "Chunk.h"
#include "Perlin.h"
#include "World.h"

void Chunk::Generate(DeviceResources* deviceRes)
{
	siv::PerlinNoise noise;
	
	for (float x = 0; x < CHUNK_SIZE.x; x++) {
		for (float y = 0; y < CHUNK_SIZE.y; y++) {
			for (float z = 0; z < CHUNK_SIZE.z; z++) {
				Vector3 pos = worldPosition + Vector3(x, y, z);
				double rate = noise.noise3D_01(pos.x / 10.0, pos.y / 10.0, pos.z / 10.0);
				int cubeValue = 0;
				if (rate < 0.5) {
					cubeValue = 1;
				}
				SetCubeAtPosition(Vector3(x, y, z), cubeValue);
			}
		}
	}

}

void Chunk::GenerateMesh(DeviceResources* deviceRes, World* world) {
	vertexBuffer.Clear();
	indexBuffer.Clear();

	int sideTexture;
	for (int index = 0; index < CHUNK_CUBES_COUNT; index++) {
		if (cubes[index] == 0) continue;
		Vector3 pos = GetPositionFromIndex(index);
		sideTexture = world->GetCubeAtPosition(worldPosition + pos + Vector3::Up) != 0 ? 2 : 3;
		if (world->GetCubeAtPosition(worldPosition + pos + Vector3::Left) == 0) {
			PushFace(pos + Vector3::Forward, Vector3::Up, Vector3::Backward, Vector2(sideTexture, 0));
		}
		if (world->GetCubeAtPosition(worldPosition + pos + Vector3::Right) == 0) {
			PushFace(pos + Vector3::Right, Vector3::Up, Vector3::Forward, Vector2(sideTexture, 0));
		}
		if (world->GetCubeAtPosition(worldPosition + pos + Vector3::Up) == 0) {
			PushFace(pos + Vector3::Up, Vector3::Forward, Vector3::Right, { 0, 0 });
		}
		if (world->GetCubeAtPosition(worldPosition + pos + Vector3::Down) == 0) {
			PushFace(pos + Vector3::Zero, Vector3::Right, Vector3::Forward, { 2, 0 });
		}
		if (world->GetCubeAtPosition(worldPosition + pos + Vector3::Forward) == 0) {
			PushFace(pos + Vector3::Right + Vector3::Forward, Vector3::Up, Vector3::Left, Vector2(sideTexture, 0));
		}
		if (world->GetCubeAtPosition(worldPosition + pos + Vector3::Backward) == 0) {
			PushFace(pos + Vector3::Zero, Vector3::Up, Vector3::Right, Vector2(sideTexture, 0));
		}
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

int Chunk::GetCubeAtPosition(Vector3 pos) {
	int index = GetIndexFromPosition(pos);
	// TODO: Check neighbouring chunk
	if (index < 0 || index > CHUNK_CUBES_COUNT) return 0;
	if (
		pos.x < 0 || pos.x >= CHUNK_SIZE.x
		|| pos.y < 0 || pos.y >= CHUNK_SIZE.y
		|| pos.z < 0 || pos.z >= CHUNK_SIZE.z
	) return 0;
	return cubes[index];
}

void Chunk::SetCubeAtPosition(Vector3 pos, int cube) {
	int index = GetIndexFromPosition(pos);
	if (index < 0 || index > CHUNK_CUBES_COUNT) {
		return;
	}
	cubes[index] = cube;
}

void Chunk::PushFace(Vector3 pos, Vector3 up, Vector3 right, Vector2 tilePos) {
	float tileCount = 16;
	//pos -= Vector3(CHUNK_SIZE.x, CHUNK_SIZE.y, CHUNK_SIZE.z) / 2.0;
	Vector3 normal = right.Cross(up);
	int i0 = vertexBuffer.PushVertex(VertexLayout_PositionNormalUV(pos + up, normal, (tilePos + Vector2(0, 0)) / tileCount));
	int i1 = vertexBuffer.PushVertex(VertexLayout_PositionNormalUV(pos + right, normal, (tilePos + Vector2(1, 1)) / tileCount));
	int i2 = vertexBuffer.PushVertex(VertexLayout_PositionNormalUV(pos, normal, (tilePos + Vector2(0, 1)) / tileCount));
	int i3 = vertexBuffer.PushVertex(VertexLayout_PositionNormalUV(pos + up + right, normal, (tilePos + Vector2(1, 0)) / tileCount));
	indexBuffer.PushTriangle(i0, i1, i2);
	indexBuffer.PushTriangle(i0, i3, i1);
}

void Chunk::Draw(DeviceResources* deviceRes) {
	vertexBuffer.Apply(deviceRes);
	indexBuffer.Apply(deviceRes);
	deviceRes->GetD3DDeviceContext()->DrawIndexed(indexBuffer.Size(), 0, 0);
}
