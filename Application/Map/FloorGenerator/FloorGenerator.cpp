#include "FloorGenerator.h"

Room* FloorGenerator::GenerateFloor() {
	// 最初の部屋
	Room first;
	first.type = RoomType::Start;
	first.startDirection = Direction::South;

	// 続く部屋
	int maxDepth = 5;
	GenerateRooms(first, 1, maxDepth);

	floor_ = first;
	return &floor_;
}

void FloorGenerator::GenerateRooms(Room& room, int depth, int maxDepth) {
	if (depth >= maxDepth)
		return;

	for (int i = 0; i < 3; ++i) { // 3部屋接続
		Room next;

		if (depth + 1 >= maxDepth) {
			// 最後はゴール部屋
			next.type = RoomType::Goal;
		} else {
			// 仮置き
			if (depth == 2) {
				next.type = RoomType::Shop;
			} else {
				next.type = RoomType::Combat;
			}
		}

		// 再帰させて追加
		GenerateRooms(next, depth + 1, maxDepth);
		room.nextRooms.push_back(std::move(next));
	}
}

RoomType FloorGenerator::SelectRoomType() {
	auto& ctx = GameContext::GetInstance();
	float r = (ctx.RandomFloat(0, 100));
	if (r < 80) {
		return RoomType::Combat;
	} else {
		return RoomType::Shop;
	}
}

void FloorGenerator::LoadRoom(Room* room, Direction enterDir) {
	auto& ctx = GameContext::GetInstance();
	auto& scene = ctx.Scene();

	int num = ctx.RandomInt(1, int(roomNumberRange[int(room->type)]));
	std::string roomName;
	switch (room->type) {
	case RoomType::Start:
		roomName = "startRoom";
		break;

	case RoomType::Combat:
		roomName = "combatRoom";
		break;

	case RoomType::Shop:
		roomName = "shopRoom";
		break;

	case RoomType::Event:
		roomName = "eventRoom";
		break;

	case RoomType::Goal:
		roomName = "goalRoom";
	}

	room->connector.clear();
	room->startDirection = GetOpposite(enterDir);
	room->startPos = {};

	// 部屋生成
	scene.SceneLoad("Resources/Debug/SceneEditor/" + roomName + std::to_string(num) + ".json");

	// 接続箇所を検索
	std::vector<Model*> models;
	for (auto& obj : scene.GetCurrentScene()->GetObjects()) {
		if (dynamic_cast<Model*>(obj)) {
			auto* model = dynamic_cast<Model*>(obj);
			models.push_back(model);
		}
	}

	// 出入口
	for (auto& model : models) {
		if (model->tag == "westConnector" ||
			model->tag == "eastConnector" ||
			model->tag == "southConnector" ||
			model->tag == "northConnector") {
			auto transform = model->GetTransform();
			
			// 出入口の方向
			Direction dir;
			if (model->tag == "westConnector") {
				dir = Direction::West;
			} else if (model->tag == "eastConnector") {
				dir = Direction::East;
			} else if (model->tag == "southConnector") {
				dir = Direction::South;
			} else if (model->tag == "northConnector") {
				dir = Direction::North;
			}

			// スタート地点
			if (dir == room->startDirection) {
				room->startPos = ToXZ(transform.translate);
				continue;
			} else if (room->startPos.x == 0 && room->startPos.y == 0) {
				room->startPos = ToXZ(transform.translate);
			}

			// 出入口を作成
			if (room->connector.size() < room->nextRooms.size()) {
				RoomConnector connector;
				connector.collider = { ToXZ(transform.translate - transform.scale * 0.5f), ToXZ(transform.translate + transform.scale * 0.5f) };
				connector.connectedRoom = &room->nextRooms[room->connector.size()];
				connector.direction = dir;
				room->connector.push_back(connector);
			}
		}
	}
}

Direction FloorGenerator::GetOpposite(Direction direction) {
	switch (direction) {
	case Direction::North: return Direction::South;
	case Direction::East:  return Direction::West;
	case Direction::South: return Direction::North;
	case Direction::West:  return Direction::East;
	}
	return Direction::South;
}

