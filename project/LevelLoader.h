#pragma once
#include <string>
#include <vector>
#include "2025_CG2_DirectX/engine/Utility/Math/Transform.h"
#include "externals/nlohmann_json/json.hpp"

struct LevelData {
	struct ObjectData {
		std::string fileName;
		struct Transform transform;
	};

	std::vector<ObjectData> objects;
};

class LevelLoader
{
public:
	LevelData* LoadLevel(const std::string& fileName);

private:
	void LoadObjectRecursive(const nlohmann::json& object, LevelData* levelData);

	const std::string kDefaultBaseDirectory = "resources/levels/";
	const std::string kExtension = ".json";
};
