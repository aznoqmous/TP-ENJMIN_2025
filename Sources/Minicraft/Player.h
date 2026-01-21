#pragma once

using namespace DirectX::SimpleMath;

class Player {
public:
	Vector3 position;
	Player(Vector3 position): position(position) {}
};