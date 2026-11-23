#pragma once
#include <optional>
#include <string>
#include <utility>
#include <vector>
#include "Map/Room/RoomData.h"

// フロア設定ファイル(FloorEditorが編集)
constexpr const char* kFloorConfigPath = "Resources/Data/Floor/FloorConfig.json";

// ゴールまでの移動回数の上限
constexpr int kMaxGoalDepth = 20;

// 階層で種類を固定する部屋(ボスや確定ショップなど)
struct FixedRoom {
	int depth = 1;
	RoomType type = RoomType::Shop;
};

// ランダムに選ばれる部屋タイプ1つ分の設定
struct RoomTypeRate {
	RoomType type = RoomType::Combat;
	float weight = 0.0f;           // 出現の重み
	float repeatMultiplier = 1.0f; // 直前の部屋が同じタイプのとき、重みに掛ける倍率(0なら連続しない)
	int minDepth = 1;              // この階層より前には出さない
};

// 報酬の確率(部屋タイプごと)。数値は割合ではなく重み
struct RewardRate {
	// 報酬の種類
	float money = 0.0f;
	float weapon = 0.0f;
	float passive = 0.0f;

	// 武器のレアリティ
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

	// 階層で種類を固定する部屋(既定値は、FloorGenerator の従来の挙動と同じ)
	std::vector<FixedRoom> fixedRooms = {
		{ 1, RoomType::Combat },
		{ 5, RoomType::Shop },
		{ 9, RoomType::Boss },
	};

	// 部屋タイプの確率
	std::vector<RoomTypeRate> roomRates = {
		{ RoomType::Combat, 81.0f, 1.0f, 1 },
		{ RoomType::Elite,   9.0f, 1.0f, 1 },
		{ RoomType::Shop,   10.0f, 1.0f, 1 },
		{ RoomType::Event,   0.0f, 1.0f, 1 },
	};

	// 報酬の確率
	RewardRate combatReward = { 40, 40, 20,  70, 25, 5, 0,  30 };
	RewardRate eliteReward  = { 20, 40, 40,  10, 50, 30, 10,  70 };
	RewardRate bossReward   = { 0, 0, 1,  0, 0, 0, 0,  30 }; // ボスはパッシブ固定

	// その階層で固定されている種類(固定がなければ空)
	std::optional<RoomType> FixedTypeAt(int depth) const;

	// 部屋タイプごとの実際の確率(roomRates と同じ並び。合計1、抽選できる物がなければ全て0)。
	// depth: これから決める部屋の階層 / parent: 直前の部屋のタイプ
	std::vector<float> RoomTypeProbabilities(int depth, RoomType parent) const;

	// 階層 depth の部屋のタイプを決める。ゴール → 固定 → ランダムの順。roll は 0以上1未満
	RoomType RoomTypeAt(int depth, RoomType parent, float roll) const;

	// 報酬を決める。rollKind: 種類用 / rollRarity: レアリティ用(どちらも 0以上1未満)
	RewardPlan ChooseReward(RoomType type, float rollKind, float rollRarity) const;

	const RewardRate& RewardOf(RoomType type) const;
	RewardRate& RewardOf(RoomType type) { return const_cast<RewardRate&>(std::as_const(*this).RewardOf(type)); }
};

// 部屋タイプの名前(ファイルのキー・表示用・部屋ファイルの接頭辞)
const char* RoomTypeKey(RoomType type);                  // "Combat" など
const char* RoomTypeLabel(RoomType type);                // 「戦闘」など
const char* RoomFilePrefix(RoomType type);               // "combatRoom" など(+ 番号 + .json)。強敵はボスの部屋を使う
bool ParseRoomType(const std::string& text, RoomType& out);
