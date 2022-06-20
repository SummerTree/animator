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
	EntitiesModule::load(nlohmann::json& reader, std::shared_ptr<octoon::AssetBundle>& ab) noexcept(false)
	{
		if (reader.contains("scene"))
		{
			octoon::GameObjects objects_;

			for (auto& it : reader["scene"])
			{
				auto object = octoon::AssetBundle::instance()->loadAsset<octoon::GameObject>(it.get<std::string>());
				if (object)
					objects_.push_back(std::move(object));
			}

			this->objects = std::move(objects_);
		}
	}

	void 
	EntitiesModule::save(nlohmann::json& writer, std::shared_ptr<octoon::AssetBundle>& ab) noexcept(false)
	{
		nlohmann::json sceneJson;

		for (auto& it : this->objects.getValue())
		{
			auto package = ab->createAsset(it);
			if (package.is_object())
				sceneJson.push_back(package["uuid"].get<std::string>());
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