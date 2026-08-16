#include "FloorManager.h"

void FloorManager::Initialize() {
	generator_ = std::make_unique<FloorGenerator>();
	currentRoom_ = generator_->GenerateFloor();
	generator_->LoadRoom(currentRoom_, Direction::North);
}

void FloorManager::LoadNextRoom(Direction enterDir) {
	for (auto& connector : currentRoom_->connector) {
		if (connector.direction == enterDir) {
			generator_->LoadRoom(connector.connectedRoom, enterDir);

			// 現在の部屋を移動
			currentRoom_ = connector.connectedRoom;
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
	for (auto& c : currentRoom_->connector) {
		// 方向が違えば追加
		if (c.direction != currentRoom_->startDirection) {
			connectors.push_back(c);
		}
	}

	return connectors;
}