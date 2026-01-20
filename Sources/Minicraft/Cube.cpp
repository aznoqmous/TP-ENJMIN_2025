#include "pch.h"
#include "Cube.h"

void Cube::Generate(DeviceResources* deviceRes) {
	PushFace(Vector3::Zero, Vector3::Up, Vector3::Right, {3, 0});
	PushFace(Vector3::Right, Vector3::Up, Vector3::Forward, {3, 0});
	PushFace(Vector3::Right + Vector3::Forward, Vector3::Up, Vector3::Left, {3, 0});
	PushFace(Vector3::Forward, Vector3::Up, Vector3::Backward, {3, 0});
	PushFace(Vector3::Up, Vector3::Forward, Vector3::Right, {2, 0});
	PushFace(Vector3::Zero, Vector3::Right, Vector3::Forward, {0, 0});

	vertexBuffer.Create(deviceRes);
	indexBuffer.Create(deviceRes);
}

void Cube::PushFace(Vector3 pos, Vector3 up, Vector3 right, Vector2 tilePos) {
	float tileCount = 16;
	pos -= {0.5, 0.5, -0.5};
	int i0 = vertexBuffer.PushVertex(VertexLayout_PositionUV(pos + up, (tilePos + Vector2(0, 1)) / tileCount));
	int i1 = vertexBuffer.PushVertex(VertexLayout_PositionUV(pos + right, (tilePos + Vector2(1, 0)) / tileCount));
	int i2 = vertexBuffer.PushVertex(VertexLayout_PositionUV(pos, (tilePos + Vector2(0, 0)) / tileCount));
	int i3 = vertexBuffer.PushVertex(VertexLayout_PositionUV(pos + up + right, (tilePos + Vector2(1, 1)) / tileCount));
	indexBuffer.PushTriangle(i0, i1, i2);
	indexBuffer.PushTriangle(i0, i3, i1);

}

void Cube::Draw(DeviceResources* deviceRes) {
	modelMatrix = Matrix::CreateTranslation(position);
	vertexBuffer.Apply(deviceRes);
	indexBuffer.Apply(deviceRes);
	deviceRes->GetD3DDeviceContext()->DrawIndexed(indexBuffer.Size(), 0, 0);
}

Matrix Cube::GetModelMatrix() {
	return modelMatrix;
}