#pragma once
#include <string>
#include <vector>
#include "../Utility/Math/Transform.h"
#include "../../../externals/nlohmann_json/json.hpp"

// 自キャラの生成データ
struct PlayerSpawnData {
	// 平行移動
	Vector3 translation;
	// 回転角
	Vector3 rotation;
};

// 敵キャラの生成データ
struct EnemySpawnData {
	// ファイル名
	std::string fileName;
	// 平行移動
	Vector3 translation;
	// 回転角
	Vector3 rotation;
};

struct CoinSpawnData {
	Vector3 translation;
};

struct LevelData {
	struct ObjectData {
		std::string fileName;
		struct EulerTransform transform;
	};

	std::vector<ObjectData> objects;
	std::vector<PlayerSpawnData> players;
	std::vector<EnemySpawnData> enemies;
	std::vector<CoinSpawnData> coins;
};

class LevelLoader
{
public:
	std::unique_ptr<LevelData> LoadLevel(const std::string& fileName);

private:
	void LoadObjectRecursive(const nlohmann::json& object, LevelData* levelData);

	const std::string kDefaultBaseDirectory = "resources/levels/";
	const std::string kExtension = ".json";
};
