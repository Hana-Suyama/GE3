#pragma once
#include <string>
#include <vector>
#include "../Utility/Math/Transform.h"
#include "../../../externals/nlohmann_json/json.hpp"

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
	std::unique_ptr<LevelData> LoadLevel(const std::string& fileName);

private:
	void LoadObjectRecursive(const nlohmann::json& object, LevelData* levelData);

	const std::string kDefaultBaseDirectory = "resources/levels/";
	const std::string kExtension = ".json";
};
