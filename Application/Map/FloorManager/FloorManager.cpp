#include "FloorManager.h"
#include "RandomSettings/RandomSettings.h"
#include "Objects/Character/Enemy/EnemyManager/EnemyManager.h"

void FloorManager::Initialize() {
	generator_ = std::make_unique<FloorGenerator>();
	currentRoom_ = generator_->GenerateFloor(kMaxDepth);
	generator_->LoadRoom(currentRoom_, Direction::North);
}

void FloorManager::Update(EnemyManager* enemyManager) {
	switch (currentRoom_->state) {
	case RoomState::Entered:
		if (!enemyManager->GetEnemies().empty()) {
			// 敵が出現したら移行
			currentRoom_->state = RoomState::InCombat;
		} else if (currentRoom_->plan.enemies.empty()) {
			// 敵がいなかったらクリア扱い
			currentRoom_->state = RoomState::Cleared;
		}
		break;
	case RoomState::InCombat:
		if (enemyManager->GetEnemies().empty()) {
			// 敵がいなくなったら移行
			currentRoom_->state = RoomState::Cleared;
		}
		break;
	}
}

void FloorManager::LoadNextRoom(Direction enterDir) {
	for (auto& connector : currentRoom_->connector) {
		if (connector.direction == enterDir) {
			generator_->LoadRoom(connector.connectedRoom, enterDir);

			// 現在の部屋を移動
			currentRoom_ = connector.connectedRoom;
			currentDepth_++;
			return;
		}
	}
}

Vector2 FloorManager::GetStartPos() {
	if (!currentRoom_) { return {}; }
	return currentRoom_->startPos;
}

std::vector<RoomConnector> FloorManager::GetConnector() {
	std::vector<RoomConnector> connectors;
	if (currentRoom_) {
		for (auto& c : currentRoom_->connector) {
			// 方向が違えば追加
			if (c.direction != currentRoom_->startDirection) {
				connectors.push_back(c);
			}
		}
	}

	return connectors;
}

void FloorManager::PlanNextRooms(RandomSettings* random) {
	if (!currentRoom_ || !random) { return; }

	auto& children = currentRoom_->nextRooms;
	if (children.empty() || children[0].plan.planned) { return; } // 確定済み

	const int childDepth = currentDepth_ + 1;
	for (auto& child : children) {
		child.plan = random->PlanRoom(child.type, childDepth);
	}
}