#pragma once
#include "GameCommon.h"
#include "Timer/Timer.h"

class Character;

enum class CharacterReactionState {
	None,
	Fall,
	Stun
};

// キャラクター全体の反応状態
class CharacterReaction {
public:
	virtual ~CharacterReaction() = default;
	float GetRemaining() const { return timer_.GetRemaining(); }

	/// <summary>
	/// 状態の更新
	/// </summary>
	// true を返したら反応状態を終了する
	virtual bool Update(Character* self) = 0;

	// 現在State
	virtual CharacterReactionState GetCurrentState() const = 0;
protected:
	Timer timer_;
};

