#pragma once
#include <vector>
#include <Externals/nlohmann/json.hpp>
#include "Objects/Character/Enemy/EnemyData.h"
#include "Data/DataIssue.h"

/// <summary>
/// 敵データとJSONの変換(ゲームの読み込みとエディタの保存で共通)
/// </summary>
namespace EnemySerializer {

	// JSON(1体分)から読み込む。欠けている項目は既定値になる
	EnemyData FromJson(const nlohmann::json& j);

	// 保存用のJSON。キーの並びは既存のファイルと同じ
	nlohmann::ordered_json ToJson(const EnemyData& enemy);

	// 設定の問題点を調べる(武器の存在など、他のデータとの照合はエディタ側)
	std::vector<DataIssue> Validate(const EnemyData& enemy);
}
