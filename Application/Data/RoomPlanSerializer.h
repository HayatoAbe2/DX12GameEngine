#pragma once
#include <string>
#include <vector>
#include <Externals/nlohmann/json.hpp>
#include "Map/Room/RoomData.h"
#include "Data/DataIssue.h"

/// <summary>
/// 部屋の予定(RoomPlan)とJSONの変換。
/// 1部屋分(FromJson / ToJson)と、フロアの木全体(TreeToJson / ApplyTree)を扱う。
/// </summary>
namespace RoomPlanSerializer {

	// JSON(1部屋分の plan)から読み込む。欠けている項目は既定値、不明な文字列は issues に警告を積む
	RoomPlan FromJson(const nlohmann::json& j, std::vector<DataIssue>* issues = nullptr, const std::string& where = "");

	// 保存用のJSON(1部屋分の plan)
	nlohmann::ordered_json ToJson(const RoomPlan& plan);

	// 内容の問題点を調べる(敵の名前の照合には EnemyManager::GetEnemyNames() を渡す)
	std::vector<DataIssue> Validate(const RoomPlan& plan, RoomType type, const std::vector<std::string>& enemyNames);

	// フロア全体を保存用のJSONにする。予定のある部屋までの道筋だけを出力する
	nlohmann::ordered_json TreeToJson(const Room& root);

	// 保存したJSONを木へ上書きする。next の添字で対応づける。
	// 部屋は入る直前に作る方式なので、まだ作られていない部屋はここで(型だけ入れて)作る。
	// すでに作られている部屋の出入口は変えない(数も増やさない)。maxDepth より先は作らない
	void ApplyTree(Room& root, const nlohmann::json& j, int maxDepth, std::vector<DataIssue>* issues = nullptr);

	// 木の中の予定をまとめて検証する。メッセージの先頭に "root/1/0" のような道筋が付く
	std::vector<DataIssue> ValidateTree(const Room& root, const std::vector<std::string>& enemyNames);
}
