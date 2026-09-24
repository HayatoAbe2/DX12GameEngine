#pragma once
#include "GameCommon.h"
class GameCameraController {
public:
	GameCameraController(Camera* camera);
	void Update(Vector3 center);

	Vector3 ScreenToWorldOnGround(Vector2 screenPos);

private:
	Camera* camera_ = nullptr;
	float cameraDistance_ = 20.0f;
};

