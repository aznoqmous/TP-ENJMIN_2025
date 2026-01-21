//
// Game.cpp
//

#include "pch.h"
#include "Game.h"
#include "Engine/VertexLayout.h"
#include "Engine/Buffer.h"

#include "PerlinNoise.hpp"
#include "Engine/Shader.h"
#include "Engine/Texture.h"
#include "Engine/Camera.h"
#include "Minicraft/Chunk.h"
#include "Minicraft/World.h"
#include "Minicraft/Player.h"

extern void ExitGame() noexcept;

using namespace DirectX;
using namespace DirectX::SimpleMath;

using Microsoft::WRL::ComPtr;

// Global stuff
Shader basicShader(L"basic");
Texture terrain(L"terrain");

CommonStates* commonStates;

Camera camera;

World world;
Player player(Vector3(16, 16, 16));

// Game
Game::Game() noexcept(false) {
	m_deviceResources = std::make_unique<DeviceResources>(DXGI_FORMAT_B8G8R8A8_UNORM_SRGB, DXGI_FORMAT_D32_FLOAT, 2);
	m_deviceResources->RegisterDeviceNotify(this);
	
}

Game::~Game() {
	delete commonStates;
	g_inputLayouts.clear();
}

void Game::Initialize(HWND window, int width, int height) {
	// Create input devices
	m_gamePad = std::make_unique<GamePad>();
	m_keyboard = std::make_unique<Keyboard>();
	m_mouse = std::make_unique<Mouse>();
	m_mouse->SetWindow(window);
	m_mouse->SetMode(Mouse::Mode::MODE_RELATIVE);

	// Initialize the Direct3D resources
	m_deviceResources->SetWindow(window, width, height);
	m_deviceResources->CreateDeviceResources();
	m_deviceResources->CreateWindowSizeDependentResources();

	basicShader.Create(m_deviceResources.get());

	auto device = m_deviceResources->GetD3DDevice();
	commonStates = new CommonStates(device);

	GenerateInputLayout<VertexLayout_PositionNormalUV>(m_deviceResources.get(), &basicShader);

	// TP: allouer vertexBuffer ici
	//chunk.Generate(m_deviceResources.get());
	world.Generate(m_deviceResources.get());

	camera.Create(m_deviceResources.get());
	camera.SetPosition(Vector3::Backward * 5.f);
	camera.aspectRatio = (float)width / (float)height;

	terrain.Create(m_deviceResources.get());
}

void Game::Tick() {
	// DX::StepTimer will compute the elapsed time and call Update() for us
	// We pass Update as a callback to Tick() because StepTimer can be set to a "fixed time" step mode, allowing us to call Update multiple time in a row if the framerate is too low (useful for physics stuffs)
	m_timer.Tick([&]() { Update(m_timer); });

	Render();
}

Vector2 mousePos;
// Updates the world.
void Game::Update(DX::StepTimer const& timer) {
	auto const kb = m_keyboard->GetState();
	auto const ms = m_mouse->GetState();

	// add kb/mouse interact here

	Vector3 movement;
	if (kb.IsKeyDown(Keyboard::Keys::Q)) movement += Vector3::Left;
	if (kb.IsKeyDown(Keyboard::Keys::D)) movement += Vector3::Right;
	if (kb.IsKeyDown(Keyboard::Keys::Z)) movement += Vector3::Forward;
	if (kb.IsKeyDown(Keyboard::Keys::S)) movement += Vector3::Backward;
	if (kb.IsKeyDown(Keyboard::Keys::Space)) movement += Vector3::Up;
	if (kb.IsKeyDown(Keyboard::Keys::LeftShift)) movement += Vector3::Down;

	camera.SetPosition(camera.position + Vector3::Transform(movement, camera.rotation) * timer.GetElapsedSeconds() * 10.0);

	Vector2 mouseDelta = mousePos - Vector2(ms.x, ms.y);
	mousePos = Vector2(ms.x, ms.y);
	//camera.rotation *= Quaternion::CreateFromYawPitchRoll(mouseDelta.x / 100.0, mouseDelta.y / 100.0, 0);
	camera.rotation *= Quaternion::CreateFromAxisAngle(camera.right, -ms.y / 1000.0);
	camera.SetRotation(camera.rotation);
	camera.rotation *= Quaternion::CreateFromAxisAngle(Vector3::Up, -ms.x / 1000.0);
	camera.SetRotation(camera.rotation);

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

	context->ClearRenderTargetView(renderTarget, ColorsLinear::CornflowerBlue);
	context->ClearDepthStencilView(depthStencil, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	context->RSSetViewports(1, &viewport);
	context->OMSetRenderTargets(1, &renderTarget, depthStencil);

	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	ApplyInputLayout<VertexLayout_PositionNormalUV>(m_deviceResources.get());

	context->RSSetState(commonStates->CullNone());
	//context->RSSetState(commonStates->Wireframe());
	
	basicShader.Apply(m_deviceResources.get());

	terrain.Apply(m_deviceResources.get());

	camera.ApplyCamera(m_deviceResources.get());

	//modelBuffer.data.modelMatrix = Matrix::CreateTranslation(Vector3::Forward * 100.f);
	//modelBuffer.UpdateBuffer(m_deviceResources.get());
	//modelBuffer.ApplyToVS(m_deviceResources.get(), 0);

	world.Draw(m_deviceResources.get());
	//chunk.Draw(m_deviceResources.get());

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

	camera.aspectRatio = (float)width / (float)height;
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
