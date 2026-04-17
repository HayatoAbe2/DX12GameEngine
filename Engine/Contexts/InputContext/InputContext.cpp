#include "InputContext.h"

InputContext::InputContext(InputSystem* input) {
	input_ = input;
}

bool InputContext::IsTrigger(uint8_t keyNumber) {
	return input_->IsTrigger(keyNumber);
}

bool InputContext::IsPress(uint8_t keyNumber) {
	return input_->IsPress(keyNumber);
}

bool InputContext::IsRelease(uint8_t keyNumber) {
	return input_->IsRelease(keyNumber);
}

bool InputContext::IsClickLeft() {
	return input_->IsClickLeft();
}

bool InputContext::IsClickRight() {
	return input_->IsClickRight();
}

bool InputContext::IsClickWheel() {
	return input_->IsClickWheel();
}

bool InputContext::IsTriggerLeftClick() {
	return input_->IsTriggerLeftClick();
}

bool InputContext::IsTriggerRightClick() {
	return input_->IsTriggerRightClick();
}

bool InputContext::IsTriggerMouseWheel() {
	return input_->IsTriggerMouseWheel();
}

Vector3 InputContext::GetMouseMove() {
	return input_->GetMouseMove();
}

Vector2 InputContext::GetMousePosition() {
	return input_->GetMousePosition();
}

bool InputContext::IsControllerPress(uint8_t buttonNumber) {
	return input_->IsControllerPress(buttonNumber);
}

/// <summary>
/// 左スティック
/// </summary>
/// <returns>-1~1</returns>
Vector2 InputContext::GetLeftStick() {
	return input_->GetLeftStick();
}

/// <summary>
/// 右スティック
/// </summary>
/// <returns>-1~1</returns>
Vector2 InputContext::GetRightStick() {
	return input_->GetRightStick();
}