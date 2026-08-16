#pragma once
#include "Map/FloorGenerator/FloorGenerator.h"

class FloorManager {
public:
	void Initialize();
	void LoadNextRoom(Direction exitDir);
	Vector2 GetStartPos();

	std::vector<RoomConnector> GetConnector();
private:
	std::unique_ptr<FloorGenerator> generator_ = nullptr;
	Room* currentRoom_;
};

