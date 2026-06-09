#pragma once
#include "Engine/Io/InputSystem/InputSystem.h"

// キーボード
class KeyboardContext {
public:
	explicit KeyboardContext(InputSystem* input) : input_(input) {}

	bool IsTrigger(uint8_t keyNumber);
	bool IsPress(uint8_t keyNumber);
	bool IsRelease(uint8_t keyNumber);

private:
	InputSystem* input_;
};

// マウス
class MouseContext {
public:
	explicit MouseContext(InputSystem* input) : input_(input) {}

	bool IsTrigger(const MouseButton& button);
	bool IsPress(const MouseButton& button);
	bool IsRelease(const MouseButton& button);
	Vector3 GetMouseMove();
	Vector2 GetPosition();

private:
	InputSystem* input_;
};

// コントローラー
class GamepadContext {
public:
	explicit GamepadContext(InputSystem* input) : input_(input) {}

	bool IsTrigger(WORD button);
	bool IsPress(WORD button);
	bool IsRelease(WORD button);
	Vector2 GetLeftStick();
	Vector2 GetRightStick();
	float GetLTrigger();
	float GetRTrigger();
	bool IsConnected();

private:
	InputSystem* input_;
};

class InputContext {
public:
	explicit InputContext(InputSystem* input)
		: keyboard(input),
		mouse(input),
		gamepad(input) {
	}

	KeyboardContext keyboard;
	MouseContext mouse;
	GamepadContext gamepad;

private:
};

