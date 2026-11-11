#pragma once
#include <string>
#include <vector>
#include <Externals/nlohmann/json.hpp>
#include "Objects/Weapon/WeaponStatus.h"
#include "Data/DataIssue.h"

/// <summary>
/// 武器データとJSONの変換(ゲームの読み込みとエディタの保存で共通)
/// </summary>
namespace WeaponSerializer {

	// JSON(1武器分)から読み込む。欠けている項目は既定値になる
	WeaponData FromJson(const std::string& name, const nlohmann::json& j);

	// 保存用のJSON。キーの並びは既存のファイルと同じ
	nlohmann::ordered_json ToJson(const WeaponData& weapon);

	// 設定の問題点を調べる
	std::vector<DataIssue> Validate(const WeaponData& weapon);

	// 敵用の武器か(名前の末尾が _Enemy)
	bool IsEnemyWeapon(const std::string& name);

	// 目安の射程(60FPS前提。軌道弾は軌道の半径、上限は kMaxRange)
	float EstimateRange(const WeaponData& weapon);
	constexpr float kMaxRange = 9999.0f;
}
