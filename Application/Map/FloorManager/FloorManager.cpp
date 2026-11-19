#include "FloorManager.h"
#include "RandomSettings/RandomSettings.h"
#include "Objects/Character/Enemy/EnemyManager/EnemyManager.h"
#include "Data/FloorConfigSerializer.h"
#include "Data/RoomPlanSerializer.h"
#include "Data/JsonFormat.h"
#include <filesystem>
#include <fstream>

void FloorManager::Initialize() {
	// 設定ファイルを読み直す(エラーのある設定は使わず、今の設定のまま)
	std::vector<DataIssue> issues;
	ReloadConfig(&issues);
	for (const auto& issue : issues) {
		OutputDebugStringA(((issue.isError ? "[FloorConfig error] " : "[FloorConfig warn] ") + issue.message + "\n").c_str());
	}

	maxDepth_ = config_.goalDepth; // 作る木に合わせて固定する
	currentDepth_ = 0;
	generator_ = std::make_unique<FloorGenerator>();
	currentRoom_ = generator_->GenerateFloor(config_);
	rootRoom_ = currentRoom_;
	generator_->LoadRoom(currentRoom_, Direction::North);
}

bool FloorManager::ReloadConfig(std::vector<DataIssue>* issues) {
	FloorConfig loaded;
	if (!FloorConfigSerializer::Load(kFloorConfigPath, loaded)) {
		if (issues) { issues->push_back({ true, "FloorConfig.json を読めません(前の設定のまま)" }); }
		return false;
	}

	bool ok = true;
	for (auto& issue : FloorConfigSerializer::Validate(loaded)) {
		if (issue.isError) { ok = false; }
		if (issues) { issues->push_back(std::move(issue)); }
	}
	if (!ok) { return false; }

	config_ = std::move(loaded);
	return true;
}

bool FloorManager::SaveFloorPlan(const std::string& path) const {
	if (!rootRoom_) { return false; }

	const std::filesystem::path p(path);
	std::error_code ec;
	if (p.has_parent_path()) { std::filesystem::create_directories(p.parent_path(), ec); }

	std::ofstream file(p);
	if (!file.is_open()) { return false; }
	file << DumpJson(RoomPlanSerializer::TreeToJson(*rootRoom_)) << '\n';
	return file.good();
}

bool FloorManager::LoadFloorPlan(const std::string& path, std::vector<DataIssue>* issues) {
	if (!rootRoom_) { return false; }

	std::ifstream file(path);
	if (!file.is_open()) { return false; } // ファイルがなければ何もしない

	// 読む(読めなければdiscarded)
	const nlohmann::json j = nlohmann::json::parse(file, nullptr, false);
	if (j.is_discarded() || !j.is_object()) {
		if (issues) { issues->push_back({ true, "JSONとして読めません: " + path }); }
		return false;
	}

	RoomPlanSerializer::ApplyTree(*rootRoom_, j, issues);
	return true;
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

	const int childDepth = currentDepth_ + 1;
	for (auto& child : currentRoom_->nextRooms) {
		if (child.plan.planned) { continue; } // 抽選済み、またはファイルで指定済み
		child.plan = random->PlanRoom(child.type, childDepth, config_);
	}
}