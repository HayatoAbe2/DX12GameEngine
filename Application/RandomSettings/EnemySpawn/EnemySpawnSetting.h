#pragma once
#include <vector>
#include <string>

/// <summary>
/// 敵の出現データ
/// </summary>
struct EnemySpawn {
	std::string name;
	float weight = 0;
};

/// <summary>
/// 敵の出現確率設定
/// </summary>
class EnemySpawnSetting {
public:
	EnemySpawnSetting();

	// ファイル読み込み
	void Load(std::string filePath);

	const std::vector<EnemySpawn>& GetSpawnData() { return spawnData_; }

private:
	// 出現データ
	std::vector<EnemySpawn> spawnData_;
};

