#pragma once
#include "Map/FloorGenerator/FloorGenerator.h"

class FloorManager {
public:
	void Initialize();
	void LoadNextRoom(Direction exitDir);
	Vector2 GetStartPos();
	void Reset();

	int GetCurrentDepth() { return currentDepth_; }
	std::vector<RoomConnector> GetConnector();

	const int kMaxDepth = 10;
private:
	std::unique_ptr<FloorGenerator> generator_ = nullptr;
	Room* currentRoom_;
	int currentDepth_ = 0;
};

