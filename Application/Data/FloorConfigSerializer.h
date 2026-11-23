#pragma once
#include <string>
#include <vector>
#include <Externals/nlohmann/json.hpp>
#include "Map/FloorConfig.h"
#include "Data/DataIssue.h"

/// <summary>
/// フロア設定とJSONの変換(ゲームの読み込みとエディタの保存で共通)
/// </summary>
namespace FloorConfigSerializer {

	// JSONから読み込む。欠けている項目・不正な項目は既定値のまま
	FloorConfig FromJson(const nlohmann::json& j);

	// 保存用のJSON。固定の部屋は階層順に並べる
	nlohmann::ordered_json ToJson(const FloorConfig& config);

	// 設定の問題点を調べる。エラーのある設定はゲームでは使わない(階層の範囲外・重複・確率が全て0など)
	std::vector<DataIssue> Validate(const FloorConfig& config);

	// ファイルから読む。ファイルが無ければ既定値で成功、壊れていれば false(out は変えない)
	bool Load(const std::string& path, FloorConfig& out);

	// ファイルへ保存する(フォルダが無ければ作る)
	bool Save(const std::string& path, const FloorConfig& config);
}
