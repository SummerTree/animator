#include "entities_module.h"
#include <octoon/asset_bundle.h>
#include <octoon/asset_database.h>
#include <octoon/animator_component.h>
#include <octoon/skinned_mesh_renderer_component.h>
#include <octoon/mesh_animation_component.h>
#include <octoon/transform_component.h>
#include <filesystem>

namespace unreal
{
	EntitiesModule::EntitiesModule() noexcept
	{
		this->reset();
	}

	EntitiesModule::~EntitiesModule() noexcept
	{
	}

	void
	EntitiesModule::reset() noexcept
	{
		this->objects.getValue().clear();
	}

	void 
	EntitiesModule::load(nlohmann::json& reader) noexcept(false)
	{
		if (reader.contains("scene"))
		{
			octoon::GameObjects objects_;

			for (auto& it : reader["scene"])
			{
				auto uuid = it.get<std::string>();
				auto assetPath = octoon::AssetDatabase::instance()->getAssetPath(uuid);
				if (!assetPath.empty())
				{
					auto object = octoon::AssetDatabase::instance()->loadAssetAtPath<octoon::GameObject>(assetPath);
					if (object)
						objects_.push_back(std::move(object));
				}
			}

			this->objects = std::move(objects_);
		}
	}

	void 
	EntitiesModule::save(nlohmann::json& writer) noexcept(false)
	{
		nlohmann::json sceneJson;

		for (auto& it : this->objects.getValue())
		{
			if (octoon::AssetDatabase::instance()->contains(it))
				sceneJson.push_back(octoon::AssetDatabase::instance()->getAssetGuid(it));
			else
			{
				auto uuid = octoon::make_guid();
				auto path = std::filesystem::path("Assets/Models").append(uuid).append(uuid + ".prefab");
				octoon::AssetDatabase::instance()->createAsset(it, path);
				sceneJson.push_back(octoon::AssetDatabase::instance()->getAssetGuid(path));
			}
		}

		writer["scene"] = std::move(sceneJson);
	}

	void
	EntitiesModule::disconnect() noexcept
	{
		this->enable.disconnect();
		this->objects.disconnect();
	}
}