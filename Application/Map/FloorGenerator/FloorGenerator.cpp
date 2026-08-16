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
			if (depth == 3) {
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
	std::vector<InstancedModel*> models;
	for (auto& obj : scene.GetCurrentScene()->GetObjects()) {
		if (dynamic_cast<InstancedModel*>(obj)) {
			auto* model = dynamic_cast<InstancedModel*>(obj);
			models.push_back(model);
		}
	}

	for (auto& model : models) {
		if (model->tag == "roomConnector") {
			auto transforms = model->GetTransforms();
			int connectorNum = std::clamp(model->GetNumInstance(), 1, 4);

			for (int i = 0; i < connectorNum; ++i) {
				if (transforms[i].scale == Vector3{ 0,0,0 } || transforms[i].translate == Vector3{ 0,0,0 }) { continue; }

				Direction dir = GetDirection(transforms[i].rotate.y);
				if (dir == room->startDirection) {
					room->startPos = ToXZ(transforms[i].translate);
					continue;
				} else if (room->startPos.x == 0 && room->startPos.y == 0) {
					room->startPos = ToXZ(transforms[i].translate);
				}

				if (room->connector.size() < room->nextRooms.size()) {
					RoomConnector connector;
					connector.collider = { ToXZ(transforms[i].translate - transforms[i].scale * 0.5f), ToXZ(transforms[i].translate + transforms[i].scale * 0.5f) };
					connector.connectedRoom = &room->nextRooms[room->connector.size()];
					connector.direction = dir;
					room->connector.push_back(connector);
				}
			}
		}
	}
}

Direction FloorGenerator::GetDirection(float rotationY) {
	// -π ～ π を 0 ～ 2π に正規化
	rotationY = std::fmod(rotationY, 2.0f * float(std::numbers::pi));

	if (rotationY < 0.0f)
		rotationY += 2.0f * float(std::numbers::pi);

	int index = int(std::round(rotationY / (float(std::numbers::pi) / 2.0f))) % 4;
	switch (index) {
	case 0: return Direction::North;
	case 1: return Direction::East;
	case 2: return Direction::South;
	case 3: return Direction::West;
	}

	return Direction::North;
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