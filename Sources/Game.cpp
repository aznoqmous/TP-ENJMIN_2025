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
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

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
	camera.SetPosition(Vector3::Backward * 5.f + Vector3::Up * CHUNK_SIZE.y);
	camera.aspectRatio = (float)width / (float)height;

	terrain.Create(m_deviceResources.get());

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // IF using Docking Branch
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // IF using Docking Branch

	ImGui_ImplWin32_Init(window);
	ImGui_ImplDX11_Init(m_deviceResources->GetD3DDevice(), m_deviceResources->GetD3DDeviceContext());
}

void Game::Tick() {
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	// DX::StepTimer will compute the elapsed time and call Update() for us
	// We pass Update as a callback to Tick() because StepTimer can be set to a "fixed time" step mode, allowing us to call Update multiple time in a row if the framerate is too low (useful for physics stuffs)
	m_timer.Tick([&]() { Update(m_timer); });

	Render();
}
bool imGuiMode = false;
Vector2 mousePos;
Vector3 velocity;
bool isJumpPressed = false;
bool isGrounded = false;

// Updates the world.
void Game::Update(DX::StepTimer const& timer) {
	auto const kb = m_keyboard->GetState();
	auto const ms = m_mouse->GetState();

	// add kb/mouse interact here

	if (kb.P) imGuiMode = true;
	if (kb.M) imGuiMode = false;

	if (imGuiMode) {
		m_mouse->SetMode(Mouse::MODE_ABSOLUTE);
		world.ShowImGui(m_deviceResources.get());
	}
	else {
		m_mouse->SetMode(Mouse::MODE_RELATIVE);

		Vector3 movement;

		Vector3 cameraPosition = Vector3(round(camera.position.x), round(camera.position.y), round(camera.position.z));
		world.modelBuffer.data.selectedCube = cameraPosition + Vector3::Down; 
		BlockId blockId = world.GetCubeAtPosition(cameraPosition + Vector3::Down * 2.0);
		if (blockId == EMPTY) {
			velocity.y -= timer.GetElapsedSeconds() * 9.8 / 4.0;
			isGrounded = false;
		}
		else {
			if (velocity.y < 0) {
				velocity.y = 0;
			}
			isGrounded = true;
		}


		if (kb.IsKeyDown(Keyboard::Keys::Q)) movement += Vector3::Left;
		if (kb.IsKeyDown(Keyboard::Keys::D)) movement += Vector3::Right;
		if (kb.IsKeyDown(Keyboard::Keys::Z)) movement += Vector3::Forward;
		if (kb.IsKeyDown(Keyboard::Keys::S)) movement += Vector3::Backward;
		if (kb.IsKeyDown(Keyboard::Keys::Space) && !isJumpPressed && isGrounded && world.GetCubeAtPosition(cameraPosition - Vector3::Up) == EMPTY)
		{
			velocity.y = 1.0;
			isJumpPressed = true;
		}
		if (!kb.IsKeyDown(Keyboard::Keys::Space)) {
			isJumpPressed = false;
		}
		if (kb.IsKeyDown(Keyboard::Keys::LeftAlt)) {
			velocity.y = 1.0;
		}
		//if (kb.IsKeyDown(Keyboard::Keys::LeftShift)) movement += Vector3::Down;
		movement = Vector3::Transform(Vector3(movement.x, 0, movement.z), camera.rotation);
		movement.y = 0;

		Vector3 movementPosition = Vector3(round(camera.position.x + movement.x), round(camera.position.y + movement.y), round(camera.position.z + movement.z));
		if (world.GetCubeAtPosition(movementPosition + Vector3::Down) != EMPTY) {
			movement = Vector3::Zero;
		}
		camera.SetPosition(
			camera.position 
			+ movement * timer.GetElapsedSeconds() * 10.0
			+ Vector3(0, velocity.y, 0) * timer.GetElapsedSeconds() * 10.0
		);
		
		//camera.SetPosition(camera.position + movement * timer.GetElapsedSeconds() * 30.0);

		Vector2 mouseDelta = mousePos - Vector2(ms.x, ms.y);
		mousePos = Vector2(ms.x, ms.y);
		//camera.rotation *= Quaternion::CreateFromYawPitchRoll(mouseDelta.x / 100.0, mouseDelta.y / 100.0, 0);
		camera.rotation *= Quaternion::CreateFromAxisAngle(camera.right, -ms.y / 1000.0);
		camera.SetRotation(camera.rotation);
		camera.rotation *= Quaternion::CreateFromAxisAngle(Vector3::Up, -ms.x / 1000.0);
		camera.SetRotation(camera.rotation);

		
	}

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

	world.UpdateChunks(camera.position, m_deviceResources.get());
	world.Draw(m_deviceResources.get());
	//chunk.Draw(m_deviceResources.get());

	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	ImGui::UpdatePlatformWindows();
	ImGui::RenderPlatformWindowsDefault();

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
