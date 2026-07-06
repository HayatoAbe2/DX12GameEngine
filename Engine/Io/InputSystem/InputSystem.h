#pragma once
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#include <cstdint>
#include <wrl.h>
#include <Xinput.h>
#pragma comment(lib, "Xinput.lib")
#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")
#include "Engine/Math/MathUtils.h"

enum class MouseButton {
	// 左クリック
	Left,
	// 右クリック
	Right,
	// ホイール
	Middle
};

class InputSystem {
public:
	template <class T> using ComPtr = Microsoft::WRL::ComPtr<T>;

	InputSystem(HINSTANCE hInstance, HWND hwnd);
	~InputSystem();
	void Update();

	//
	// キー入力関連
	//

	/// <summary>
	/// キーを押した瞬間か
	/// </summary>
	/// <param name="keyNumber">キー番号</param>
	/// <returns>キーを押した瞬間のみtrue</returns>
	bool IsTrigger(uint8_t keyNumber) { return (!preKey_[keyNumber] && key_[keyNumber]); };	
	
	/// <summary>
	/// キーを押している状態か
	/// </summary>
	/// <param name="keyNumber">キー番号</param>
	/// <returns>キーが押されていればtrue</returns>
	bool IsPress(uint8_t keyNumber) { return (key_[keyNumber]); };
	
	/// <summary>
	/// キーを離した瞬間か
	/// </summary>
	/// <param name="keyNumber">キー番号</param>
	/// <returns>キーが離された瞬間のみtrue</returns>
	bool IsRelease(uint8_t keyNumber) { return (preKey_[keyNumber] && !key_[keyNumber]); };
	
	//
	// マウス入力関連
	//

	bool IsTrigger(const MouseButton& button) { return (!preMouseState_.rgbButtons[uint8_t(button)] && 0x80) && (mouseState_.rgbButtons[uint8_t(button)] && 0x80); };
	bool IsPress(const MouseButton& button) { return mouseState_.rgbButtons[uint8_t(button)] && 0x80; };
	bool IsRelease(const MouseButton& button) { return (preMouseState_.rgbButtons[uint8_t(button)] && 0x80) && (!mouseState_.rgbButtons[uint8_t(button)] && 0x80); };

	Vector3 GetMouseMove() { return { float(mouseState_.lX),float(mouseState_.lY),float(mouseState_.lZ) }; };
	Vector2 GetMousePosition() {
		POINT mousePosition;
		// マウスの位置を取得
		GetCursorPos(&mousePosition);
		// ウィンドウのクライアント座標に変換
		ScreenToClient(hwnd_, &mousePosition);
		return { float(mousePosition.x), float(mousePosition.y) };
	}

	//
	// コントローラー入力関連
	//

	bool IsControllerConnected() { return isControllerConnected_; }
	bool IsTrigger(WORD button) { return !(preControllerState_.Gamepad.wButtons & button) && (controllerState_.Gamepad.wButtons & button);}
	bool IsPress(WORD button) { return controllerState_.Gamepad.wButtons & button;}
	bool IsRelease(WORD button) { return (preControllerState_.Gamepad.wButtons & button) && !(controllerState_.Gamepad.wButtons & button);}

	Vector2 GetLeftStick() {
		// -1000~1000の範囲を-1.0f~1.0fに正規化
		Vector2 stick = {
			controllerState_.Gamepad.sThumbLX / 32767.0f,
			controllerState_.Gamepad.sThumbLY / 32767.0f
		};

		if (fabs(stick.x) < deadZone_) stick.x = 0.0f;
		if (fabs(stick.y) < deadZone_) stick.y = 0.0f;

		return stick;
	}

	Vector2 GetRightStick() {
		Vector2 stick = {
			controllerState_.Gamepad.sThumbRX / 32767.0f,
			controllerState_.Gamepad.sThumbRY / 32767.0f
		};

		if (fabs(stick.x) < deadZone_) stick.x = 0.0f;
		if (fabs(stick.y) < deadZone_) stick.y = 0.0f;

		return stick;
	}

	float GetLeftTrigger() {
		return controllerState_.Gamepad.bLeftTrigger / 255.0f;
	}

	float GetRightTrigger() {
		return controllerState_.Gamepad.bRightTrigger / 255.0f;
	}

	HWND GetHwnd() { return hwnd_; }

private:
	ComPtr<IDirectInput8> directInput_ = nullptr;
	ComPtr<IDirectInputDevice8> keyboard_ = nullptr;
	ComPtr<IDirectInputDevice8> mouse_ = nullptr;

	// キー入力状態
	BYTE preKey_[256]{};
	BYTE key_[256]{};

	// マウス入力状態
	DIMOUSESTATE preMouseState_{};
	DIMOUSESTATE mouseState_{};

	// コントローラー入力状態
	XINPUT_STATE preControllerState_{};
	XINPUT_STATE controllerState_{};
	bool isControllerConnected_ = false;

	// スティックのデッドゾーン
	const float deadZone_ = 0.1f;

	HWND hwnd_{}; // ウィンドウハンドル

};

