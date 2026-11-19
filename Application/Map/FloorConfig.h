#pragma once
#include <optional>
#include <string>
#include <utility>
#include <vector>
#include "Map/Room/RoomData.h"

// 編集するファイル
constexpr const char* kFloorConfigPath = "Resources/Data/Floor/FloorConfig.json";

// ゴールまでの移動回数の上限。部屋は 3^N 個作るので、これ以上はメモリが足りなくなる
constexpr int kMaxGoalDepth = 12;

// 部屋の種類固定の設定
struct FixedRoom {
	int depth = 1;
	RoomType type = RoomType::Shop;
};

// ランダム部屋の設定
struct RoomTypeRate {
	RoomType type = RoomType::Combat;
	float weight = 0.0f;           // 出現の重み
	float repeatMultiplier = 1.0f; // 連続出現のとき重みに掛ける倍率(0なら連続しない)
	int minDepth = 1;              // この階層から出現
};

// 報酬抽選の重み
struct RewardRate {
	// 報酬の種類
	float money = 0.0f;
	float weapon = 0.0f;
	float passive = 0.0f;

	// 武器のレアリティ(パッシブは固定のレアなので影響しない)
	float common = 0.0f;
	float rare = 0.0f;
	float epic = 0.0f;
	float legendary = 0.0f;

	// 金額
	int moneyAmount = 30;
};

/// <summary>
/// フロアの構成と確率の設定。生成・抽選のロジックもここに置く(乱数は呼び出し側から渡す)
/// </summary>
struct FloorConfig {
	// ゴールまでの移動回数(この階層の部屋がゴール)
	int goalDepth = 10;

	// 固定出現の部屋
	std::vector<FixedRoom> fixedRooms = {
		{ 1, RoomType::Combat },
		{ 5, RoomType::Shop },
		{ 9, RoomType::Boss },
	};

	// 部屋タイプの抽選設定
	std::vector<RoomTypeRate> roomRates = {
		{ RoomType::Combat, 81.0f, 1.0f, 1 },
		{ RoomType::Elite,   9.0f, 1.0f, 1 },
		{ RoomType::Shop,   10.0f, 1.0f, 1 },
		{ RoomType::Event,   0.0f, 1.0f, 1 },
	};

	// 報酬の確率(部屋タイプごと)
	RewardRate combatReward = { 40, 40, 20,  70, 25, 5, 0,  30 };
	RewardRate eliteReward  = { 20, 40, 40,  10, 50, 30, 10,  70 };
	RewardRate bossReward   = { 0, 0, 1,  0, 0, 0, 0,  30 };

	// (固定されていれば)されている種類
	std::optional<RoomType> FixedTypeAt(int depth) const;

	// 部屋タイプごとの実際の確率
	std::vector<float> RoomTypeProbabilities(int depth, RoomType parent) const;

	// 部屋のタイプを決める。固定の後ランダム
	RoomType RoomTypeAt(int depth, RoomType parent, float roll) const;

	// 報酬を決める
	RewardPlan ChooseReward(RoomType type, float rollKind, float rollRarity) const;

	// 報酬種類の決定
	const RewardRate& RewardOf(RoomType type) const;
	RewardRate& RewardOf(RoomType type) { return const_cast<RewardRate&>(std::as_const(*this).RewardOf(type)); }
};

// 部屋タイプの名前(ファイルのキー・表示用・部屋ファイルの接頭辞)
const char* RoomTypeKey(RoomType type);                  // "Combat" など
const char* RoomTypeLabel(RoomType type);                // 「戦闘」など
const char* RoomFilePrefix(RoomType type);               // "combatRoom" など(+ 番号 + .json)。強敵はボスの部屋を使う
bool ParseRoomType(const std::string& text, RoomType& out);
