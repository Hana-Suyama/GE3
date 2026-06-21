#include "LevelLoader.h"
#include <cassert>
#include <fstream>
#include "MyMath.h"

std::unique_ptr<LevelData> LevelLoader::LoadLevel(const std::string& fileName)
{
	const std::string fullpath = kDefaultBaseDirectory + fileName + kExtension;

	std::ifstream file;
	file.open(fullpath);
	if (file.fail()) {
		assert(0);
	}

	nlohmann::json deserialized;
	file >> deserialized;

	assert(deserialized.is_object());
	assert(deserialized.contains("name"));
	assert(deserialized["name"].is_string());

	const std::string name = deserialized["name"].get<std::string>();
	assert(name.compare("scene") == 0);

	assert(deserialized.contains("objects"));
	assert(deserialized["objects"].is_array());

	std::unique_ptr<LevelData> levelData = std::make_unique<LevelData>();

	for (const nlohmann::json& object : deserialized["objects"]) {
		LoadObjectRecursive(object, levelData.get());
	}	

	return levelData;
}

void LevelLoader::LoadObjectRecursive(const nlohmann::json& object, LevelData* levelData)
{
	assert(levelData);
	assert(object.is_object());
	assert(object.contains("type"));
	assert(object["type"].is_string());

	const std::string type = object["type"].get<std::string>();

	if (type.compare("MESH") == 0) {
		levelData->objects.emplace_back(LevelData::ObjectData{});
		LevelData::ObjectData& objectData = levelData->objects.back();

		if (object.contains("file_name")) {
			assert(object["file_name"].is_string());
			objectData.fileName = object["file_name"].get<std::string>();
		}

		assert(object.contains("transform"));
		const nlohmann::json& transform = object["transform"];

		objectData.transform.translate.x = (float)transform["translation"][0];
		objectData.transform.translate.y = (float)transform["translation"][2];
		objectData.transform.translate.z = (float)transform["translation"][1];

		objectData.transform.rotate.x = MyMath::DEGtoRAD(-(float)transform["rotation"][0]);
		objectData.transform.rotate.y = MyMath::DEGtoRAD(-(float)transform["rotation"][2]);
		objectData.transform.rotate.z = MyMath::DEGtoRAD(-(float)transform["rotation"][1]);

		objectData.transform.scale.x = (float)transform["scaling"][0];
		objectData.transform.scale.y = (float)transform["scaling"][2];
		objectData.transform.scale.z = (float)transform["scaling"][1];
	}

	if (object.contains("children")) {
		assert(object["children"].is_array());

		for (const nlohmann::json& child : object["children"]) {
			LoadObjectRecursive(child, levelData);
		}
	}
}
