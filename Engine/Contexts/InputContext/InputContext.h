#pragma once
#include "Engine/Io/InputSystem.h"
class InputContext {
public:
	InputContext(InputSystem* input);

	bool IsTrigger(uint8_t keyNumber);
	bool IsPress(uint8_t keyNumber);
	bool IsRelease(uint8_t keyNumber);
	bool IsClickLeft();
	bool IsClickRight();
	bool IsClickWheel();
	bool IsTriggerLeftClick();
	bool IsTriggerRightClick();
	bool IsTriggerMouseWheel();
	Vector3 GetMouseMove();
	Vector2 GetMousePosition();
	bool IsControllerPress(uint8_t buttonNumber);
	Vector2 GetLeftStick();
	Vector2 GetRightStick();

private:
	InputSystem* input_ = nullptr;
};

