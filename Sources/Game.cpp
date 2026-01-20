//
// Game.cpp
//

#include "pch.h"
#include "Game.h"
#include "Engine/VertexLayout.h"
#include "Engine/Buffer.h"

#include "PerlinNoise.hpp"
#include "Engine/Shader.h"

extern void ExitGame() noexcept;

using namespace DirectX;
using namespace DirectX::SimpleMath;

using Microsoft::WRL::ComPtr;

// Global stuff
Shader* basicShader;

CommonStates* commonStates;

struct ModelData {
	Matrix modelMatrix;
};
struct CameraData {
	Matrix viewMatrix;
	Matrix projectionMatrix;
};

VertexBuffer<VertexLayout_Position> vertexBuffer;
IndexBuffer indexBuffer;
ConstantBuffer<ModelData> modelBuffer;
ConstantBuffer<CameraData> cameraBuffer;

// Game
Game::Game() noexcept(false) {
	m_deviceResources = std::make_unique<DeviceResources>(DXGI_FORMAT_B8G8R8A8_UNORM, DXGI_FORMAT_D32_FLOAT, 2);
	m_deviceResources->RegisterDeviceNotify(this);

}

Game::~Game() {
	delete basicShader;
	delete commonStates;
	g_inputLayouts.clear();
}

void Game::Initialize(HWND window, int width, int height) {
	// Create input devices
	m_gamePad = std::make_unique<GamePad>();
	m_keyboard = std::make_unique<Keyboard>();
	m_mouse = std::make_unique<Mouse>();
	m_mouse->SetWindow(window);

	// Initialize the Direct3D resources
	m_deviceResources->SetWindow(window, width, height);
	m_deviceResources->CreateDeviceResources();
	m_deviceResources->CreateWindowSizeDependentResources();

	basicShader = new Shader(L"Basic");
	basicShader->Create(m_deviceResources.get());

	auto device = m_deviceResources->GetD3DDevice();
	commonStates = new CommonStates(device);

	GenerateInputLayout<VertexLayout_Position>(m_deviceResources.get(), basicShader);

	// TP: allouer vertexBuffer ici

	vertexBuffer.PushVertex({{-0.5,  0.5, 0}});
	vertexBuffer.PushVertex({{ 0.5, -0.5, 0}});
	vertexBuffer.PushVertex({{-0.5, -0.5, 0}});
	vertexBuffer.PushVertex({{ 0.5,  0.5, 0}});
	vertexBuffer.Create(m_deviceResources.get());
	
	indexBuffer.PushTriangle(0, 1, 2);
	indexBuffer.PushTriangle(0, 3, 1);
	indexBuffer.Create(m_deviceResources.get());
	
	/* Matrices */
	Vector3 position = Vector3::Forward;
	modelBuffer.Create(m_deviceResources.get());
	modelBuffer.data.modelMatrix = Matrix::CreateTranslation(position);

	cameraBuffer.Create(m_deviceResources.get());
	float fov = 1.0;
	float aspectRatio = (float)width / (float)height;
	float nearPlane = 0.1;
	float farPlane = 100.0;
	cameraBuffer.data.viewMatrix = Matrix::CreateLookAt(Vector3::Backward, position, Vector3::Up).Transpose();
	cameraBuffer.data.projectionMatrix = Matrix::CreatePerspectiveFieldOfView(fov, aspectRatio, nearPlane, farPlane).Transpose();
}

void Game::Tick() {
	// DX::StepTimer will compute the elapsed time and call Update() for us
	// We pass Update as a callback to Tick() because StepTimer can be set to a "fixed time" step mode, allowing us to call Update multiple time in a row if the framerate is too low (useful for physics stuffs)
	m_timer.Tick([&]() { Update(m_timer); });

	Render();
}

// Updates the world.
void Game::Update(DX::StepTimer const& timer) {
	auto const kb = m_keyboard->GetState();
	auto const ms = m_mouse->GetState();
	
	// add kb/mouse interact here
	
	if (kb.Escape)
		ExitGame();

	auto const pad = m_gamePad->GetState(0);
}

// Draws the scene.
void Game::Render() {
	// Don't try to render anything before the first Update.
	if (m_timer.GetFrameCount() == 0)
		return;

	auto context = m_deviceResources->GetD3DDeviceContext();
	auto renderTarget = m_deviceResources->GetRenderTargetView();
	auto depthStencil = m_deviceResources->GetDepthStencilView();
	auto const viewport = m_deviceResources->GetScreenViewport();

	context->ClearRenderTargetView(renderTarget, Colors::Black);
	context->ClearDepthStencilView(depthStencil, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	context->RSSetViewports(1, &viewport);
	context->OMSetRenderTargets(1, &renderTarget, depthStencil);
	
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	ApplyInputLayout<VertexLayout_Position>(m_deviceResources.get());

	context->RSSetState(commonStates->CullNone());
	//context->RSSetState(commonStates->Wireframe());
	
	basicShader->Apply(m_deviceResources.get());

	// TP: Tracer votre vertex buffer ici
	vertexBuffer.Apply(m_deviceResources.get());
	indexBuffer.Apply(m_deviceResources.get());

	modelBuffer.data.modelMatrix = Matrix::CreateScale(abs(sin(m_timer.GetTotalSeconds()) * 0.5) + 0.5).Transpose();
	modelBuffer.data.modelMatrix *= Matrix::CreateRotationZ(m_timer.GetTotalSeconds()).Transpose();
	modelBuffer.UpdateBuffer(m_deviceResources.get());
	modelBuffer.ApplyToVS(m_deviceResources.get());

	cameraBuffer.UpdateBuffer(m_deviceResources.get());
	cameraBuffer.ApplyToVS(m_deviceResources.get(), 1);


	context->DrawIndexed(indexBuffer.Size(), 0, 0);


	// envoie nos commandes au GPU pour etre afficher � l'�cran
	m_deviceResources->Present();
}


#pragma region Message Handlers
void Game::OnActivated() {}

void Game::OnDeactivated() {}

void Game::OnSuspending() {}

void Game::OnResuming() {
	m_timer.ResetElapsedTime();
}

void Game::OnWindowMoved() {
	auto const r = m_deviceResources->GetOutputSize();
	m_deviceResources->WindowSizeChanged(r.right, r.bottom);
}

void Game::OnDisplayChange() {
	m_deviceResources->UpdateColorSpace();
}

void Game::OnWindowSizeChanged(int width, int height) {
	if (!m_deviceResources->WindowSizeChanged(width, height))
		return;

	float fov = 1.0;
	float aspectRatio = (float)width / (float) height;
	float nearPlane = 0.1;
	float farPlane = 100.0;
	cameraBuffer.data.projectionMatrix = Matrix::CreatePerspectiveFieldOfView(fov, aspectRatio, nearPlane, farPlane).Transpose();
	// The windows size has changed:
	// We can realloc here any resources that depends on the target resolution (post processing etc)
}

void Game::OnDeviceLost() {
	// We have lost the graphics card, we should reset resources [TODO]
}

void Game::OnDeviceRestored() {
	// We have a new graphics card context, we should realloc resources [TODO]
}

// Properties
void Game::GetDefaultSize(int& width, int& height) const noexcept {
	width = 800;
	height = 600;
}

#pragma endregion
