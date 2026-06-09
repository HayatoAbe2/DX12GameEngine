#include "InputContext.h"

// --------------------
// キーボード
// --------------------
bool KeyboardContext::IsTrigger(uint8_t keyNumber) {
	return input_->IsTrigger(keyNumber);
}

bool KeyboardContext::IsPress(uint8_t keyNumber) {
	return input_->IsPress(keyNumber);
}

bool KeyboardContext::IsRelease(uint8_t keyNumber) {
	return input_->IsRelease(keyNumber);
}

// --------------------
// マウス
// --------------------

bool MouseContext::IsTrigger(const MouseButton& button) {
	return input_->IsTrigger(button);
}

bool MouseContext::IsPress(const MouseButton& button) {
	return input_->IsPress(button);
}

bool MouseContext::IsRelease(const MouseButton& button) {
	return input_->IsRelease(button);
}

Vector3 MouseContext::GetMouseMove() {
	return input_->GetMouseMove();
}

Vector2 MouseContext::GetPosition() {
	return input_->GetMousePosition();
}

// --------------------
// コントローラー
// --------------------
bool GamepadContext::IsTrigger(WORD button) {
	return input_->IsTrigger(button);
}

bool GamepadContext::IsPress(WORD button) {
	return input_->IsPress(button);
}

bool GamepadContext::IsRelease(WORD button) {
	return input_->IsRelease(button);
}

/// <summary>
/// 左スティック
/// </summary>
/// <returns>-1~1</returns>
Vector2 GamepadContext::GetLeftStick() {
	return input_->GetLeftStick();
}

/// <summary>
/// 右スティック
/// </summary>
/// <returns>-1~1</returns>
Vector2 GamepadContext::GetRightStick() {
	return input_->GetRightStick();
}

float GamepadContext::GetLTrigger() {
	return input_->GetLeftTrigger();
}

float GamepadContext::GetRTrigger() {
	return input_->GetRightTrigger();
}

bool GamepadContext::IsConnected() {
	return input_->IsControllerConnected();
}
