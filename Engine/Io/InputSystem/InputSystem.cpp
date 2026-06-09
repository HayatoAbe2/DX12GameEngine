#include "InputSystem.h"

InputSystem::InputSystem(HINSTANCE hInstance, HWND hwnd) {
	HRESULT hr;

	// オブジェクト生成
	hr = DirectInput8Create(hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8,
		(void**)&directInput_, nullptr);
	assert(SUCCEEDED(hr));

	///
	/// キーボード
	///

	// デバイスの生成
	hr = directInput_->CreateDevice(GUID_SysKeyboard, &keyboard_, NULL);
	assert(SUCCEEDED(hr));
	// 入力データ形式のセット
	hr = keyboard_->SetDataFormat(&c_dfDIKeyboard);
	assert(SUCCEEDED(hr));
	// 排他制御レベルのセット
	hr = keyboard_->SetCooperativeLevel(
		hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY);
	assert(SUCCEEDED(hr));

	///
	/// マウス
	///

	// デバイス生成
	hr = directInput_->CreateDevice(GUID_SysMouse, &mouse_, NULL);
	assert(SUCCEEDED(hr));
	// 入力データ形式のセット
	hr = mouse_->SetDataFormat(&c_dfDIMouse);
	assert(SUCCEEDED(hr));
	// 排他制御レベルのセット
	hr = mouse_->SetCooperativeLevel(
		hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY);
	assert(SUCCEEDED(hr));

	hwnd_ = hwnd;
}

InputSystem::~InputSystem() {
}

void InputSystem::Update() {
	// 前フレームのキー入力状態
	memcpy(preKey_, key_, sizeof(key_));
	// キーボード情報の取得開始
	keyboard_->Acquire();
	// 全キーの入力状態を取得
	keyboard_->GetDeviceState(sizeof(key_), key_);

	// 前フレームのマウス入力状態
	preMouseState_ = mouseState_;
	// マウス情報の取得開始
	mouse_->Acquire();
	// クリック状態
	mouse_->GetDeviceState(sizeof(mouseState_), &mouseState_);

	// 前フレームのコントローラー入力状態
	preControllerState_ = controllerState_;
	ZeroMemory(
		&controllerState_,
		sizeof(XINPUT_STATE));

	DWORD result =
		XInputGetState(
			0,
			&controllerState_);

	isControllerConnected_ = (result == ERROR_SUCCESS);
}