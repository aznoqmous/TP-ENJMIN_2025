#pragma once
#include "Engine/Buffer.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;

struct CameraData {
	Matrix viewMatrix;
	Matrix projectionMatrix;
};

class Camera {
private:

	CameraData data;
	ConstantBuffer<CameraData> cameraBuffer;


public:
	Vector3 position;
	Vector3 forward = Vector3::Forward;
	Vector3 up = Vector3::Up;
	Vector3 right = Vector3::Right;
	Quaternion rotation;

	float fov = DirectX::XMConvertToRadians(60);
	float aspectRatio = 1;
	float nearPlane = 0.1;
	float farPlane = 1000.0;
	void Create(DeviceResources* devRes);
	void SetPosition(Vector3 position);
	void SetRotation(Quaternion rotation);
	void ApplyCamera(DeviceResources* devRes);
	void UpdateViewMatrix();
	Matrix GetInverseMatrix();
};