#pragma once
#include "FloorGenerator/FloorGenerator.h"

class RandomSettings;
class EnemyManager;

/// <summary>
/// 部屋と進度管理
/// </summary>
class FloorManager {
public:
	void Initialize();
	void Update(EnemyManager* enemyManager);

	void LoadNextRoom(Direction exitDir);
	Vector2 GetStartPos();
	void Reset();

	int GetCurrentDepth() { return currentDepth_; }
	std::vector<RoomConnector> GetConnector();
	RoomState GetCurrentRoomState() { return currentRoom_->state; }

	// 部屋の内容生成
	void PlanNextRooms(RandomSettings* random);
	const RoomPlan& GetCurrentPlan() const {
		static const RoomPlan kEmpty;
		return currentRoom_ ? currentRoom_->plan : kEmpty;
	}

	const int kMaxDepth = 10;
private:
	std::unique_ptr<FloorGenerator> generator_ = nullptr;
	Room* currentRoom_;
	int currentDepth_ = 0;
};

