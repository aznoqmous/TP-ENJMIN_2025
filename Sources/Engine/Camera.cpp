#include "pch.h"
#include "Camera.h"

void Camera::SetPosition(Vector3 position){
	this->position = position;
	data.viewMatrix = Matrix::CreateLookAt(position, position + forward, up).Transpose();
}

void Camera::SetRotation(Quaternion rotation){
	this->rotation = rotation;
	UpdateViewMatrix();
}

void Camera::Create(DeviceResources* devRes) {
	cameraBuffer.Create(devRes);
	SetPosition(position);
	SetRotation(rotation);
}

void Camera::ApplyCamera(DeviceResources* devRes) {
	data.projectionMatrix = Matrix::CreatePerspectiveFieldOfView(fov, aspectRatio, nearPlane, farPlane).Transpose();
	cameraBuffer.data = data;
	cameraBuffer.UpdateBuffer(devRes);
	cameraBuffer.ApplyToVS(devRes, 1);
}

void Camera::UpdateViewMatrix() {
	forward = Vector3::Transform(Vector3::Forward, rotation);
	up = Vector3::Transform(Vector3::Up, rotation);
	right = Vector3::Transform(Vector3::Right, rotation);
	data.viewMatrix = Matrix::CreateLookAt(position, position + forward, up).Transpose();
}

Matrix Camera::GetInverseMatrix() {
	return data.viewMatrix.Invert();
}
