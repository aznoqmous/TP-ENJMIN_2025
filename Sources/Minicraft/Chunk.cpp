#include "pch.h"
#include "Chunk.h"
#include "Perlin.h"

void Chunk::Generate(DeviceResources* deviceRes)
{
	siv::PerlinNoise noise;
	cubes.clear();
	for (int x = 0; x < size.x; x++) {
		for (int y = 0; y < size.y; y++) {
			for (int z = 0; z < size.z; z++) {
				double rate = noise.noise3D_01(x / 10.0, y / 10.0, z / 10.0);
				if (rate < 0.5) {
					cubes[Vector3(x, y, z)] = 1;
				}
			}
		}
	}

	vertexBuffer.Clear();
	indexBuffer.Clear();

	for (auto it : cubes) {
		if (cubes.find(it.first + Vector3::Left) == cubes.end()) {
			PushFace(it.first + Vector3::Forward, Vector3::Up, Vector3::Backward, { 3, 0 });
		}
		if (cubes.find(it.first + Vector3::Right) == cubes.end()) {
			PushFace(it.first + Vector3::Right, Vector3::Up, Vector3::Forward, { 3, 0 });
		}
		if (cubes.find(it.first + Vector3::Up) == cubes.end()) {
			PushFace(it.first + Vector3::Up, Vector3::Forward, Vector3::Right, { 0, 0 });
		}
		if (cubes.find(it.first + Vector3::Down) == cubes.end()) {
			PushFace(it.first + Vector3::Zero, Vector3::Right, Vector3::Forward, { 2, 0 });
		}
		if (cubes.find(it.first + Vector3::Forward) == cubes.end()) {
			PushFace(it.first + Vector3::Right + Vector3::Forward, Vector3::Up, Vector3::Left, { 3, 0 });
		}
		if (cubes.find(it.first + Vector3::Backward) == cubes.end()) {
			PushFace(it.first + Vector3::Zero, Vector3::Up, Vector3::Right, { 3, 0 });
		}
	}

	vertexBuffer.Create(deviceRes);
	indexBuffer.Create(deviceRes);
}

void Chunk::PushFace(Vector3 pos, Vector3 up, Vector3 right, Vector2 tilePos) {
	float tileCount = 16;
	pos -= Vector3(size.x, size.y, size.z) / 2.0;
	int i0 = vertexBuffer.PushVertex(VertexLayout_PositionUV(pos + up, (tilePos + Vector2(0, 0)) / tileCount));
	int i1 = vertexBuffer.PushVertex(VertexLayout_PositionUV(pos + right, (tilePos + Vector2(1, 1)) / tileCount));
	int i2 = vertexBuffer.PushVertex(VertexLayout_PositionUV(pos, (tilePos + Vector2(0, 1)) / tileCount));
	int i3 = vertexBuffer.PushVertex(VertexLayout_PositionUV(pos + up + right, (tilePos + Vector2(1, 0)) / tileCount));
	indexBuffer.PushTriangle(i0, i1, i2);
	indexBuffer.PushTriangle(i0, i3, i1);

}

void Chunk::Draw(DeviceResources* deviceRes) {
	vertexBuffer.Apply(deviceRes);
	indexBuffer.Apply(deviceRes);
	deviceRes->GetD3DDeviceContext()->DrawIndexed(indexBuffer.Size(), 0, 0);
}
