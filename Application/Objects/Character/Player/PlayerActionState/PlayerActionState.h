#pragma once

class MapCheck;
class Player;

enum class PlayerState {
	Idle,
	Dodge
};

/// <summary>
/// プレイヤー行動状態
/// </summary>
class PlayerActionState {
public:
	virtual ~PlayerActionState() = default;
	// 状態が終了した場合 true を返す
	virtual bool Update(Player* player, MapCheck* mapCheck) = 0;
	// 現在State
	virtual PlayerState GetCurrentState() const = 0;
};
