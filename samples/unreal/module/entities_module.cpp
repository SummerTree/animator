#include "entities_module.h"
#include <octoon/pmx_loader.h>
#include <octoon/vmd_loader.h>
#include <octoon/asset_database.h>
#include <octoon/asset_bundle.h>
#include <octoon/model_importer.h>
#include <octoon/material_importer.h>
#include <octoon/animator_component.h>
#include <octoon/motion_importer.h>
#include <octoon/skinned_mesh_renderer_component.h>
#include <octoon/mesh_animation_component.h>
#include <octoon/transform_component.h>
#include <octoon/runtime/uuid.h>
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
		octoon::GameObjects objects_;

		if (reader["scene"].is_array())
		{
			for (auto& it : reader["scene"])
			{
				octoon::PMXLoadFlags flags = octoon::PMXLoadFlagBits::AllBit;
				if (it.find("materials") != it.end())
					flags = flags & ~octoon::PMXLoadFlagBits::MaterialBit;

				auto object = octoon::AssetBundle::instance()->loadAsset<octoon::GameObject>(it["model"].get<std::string>());
				if (object)
				{
					if (it.contains("transform"))
					{
						auto transform = object->getComponent<octoon::TransformComponent>();
						if (transform)
							transform->load(it["transform"]);
					}

					for (auto& animationJson : it["animation"])
					{
						if (animationJson.find("data") == animationJson.end())
							continue;

						auto animation = octoon::AssetBundle::instance()->loadAsset<octoon::Animation>(animationJson["data"].get<std::string>());
						if (animation)
						{
							auto type = animationJson["type"].get<nlohmann::json::number_unsigned_t>();
							if (type == 0)
								object->addComponent<octoon::AnimatorComponent>(std::move(animation), object->getComponent<octoon::SkinnedMeshRendererComponent>()->getTransforms());
							else
								object->addComponent<octoon::AnimatorComponent>(std::move(animation));
						}
					}

					if (it.find("materials") != it.end())
					{
						std::vector<std::shared_ptr<octoon::Material>> materials;
						materials.resize(it["materials"].size());

						for (auto& materialJson : it["materials"])
						{
							if (materialJson.find("data") == materialJson.end())
								continue;

							auto data = materialJson["data"].get<nlohmann::json::string_t>();
							auto index = materialJson["index"].get<nlohmann::json::number_unsigned_t>();
							auto material = octoon::AssetBundle::instance()->loadAsset<octoon::Material>(data);

							materials[index] = std::move(material);
						}

						auto smr = object->getComponent<octoon::SkinnedMeshRendererComponent>();
						if (smr)
							smr->setMaterials(std::move(materials));
					}

					objects_.push_back(std::move(object));
				}
			}
		}

		this->objects = std::move(objects_);
	}

	void 
	EntitiesModule::save(nlohmann::json& writer, std::shared_ptr<octoon::AssetBundle>& ab) noexcept(false)
	{
		nlohmann::json sceneJson;

		for (auto& it : this->objects.getValue())
		{
			auto modelPackage = ab->createAsset(it);
			if (modelPackage.is_object())
			{
				nlohmann::json json;
				json["model"] = modelPackage["uuid"];

				auto transform = it->getComponent<octoon::TransformComponent>();
				if (transform)
					transform->save(json["transform"]);

				for (auto& component : it->getComponents())
				{
					if (component->isA<octoon::AnimatorComponent>())
					{
						auto animation = component->downcast<octoon::AnimatorComponent>();
						if (animation->getAnimation())
						{
							auto package = ab->createAsset(animation->getAnimation());
							if (package.is_object())
							{
								nlohmann::json animationJson;
								animationJson["data"] = package["uuid"].get<std::string>();
								animationJson["type"] = animation->getAvatar().empty() ? 1 : 0;

								json["animation"].push_back(animationJson);
							}
						}
					}
				}

				auto smr = it->getComponent<octoon::SkinnedMeshRendererComponent>();
				if (smr)
				{
					auto& materials = smr->getMaterials();

					for (std::size_t i = 0; i < materials.size(); i++)
					{
						auto package = ab->createAsset(materials[i]);
						if (package.is_object())
						{
							nlohmann::json materialJson;
							materialJson["data"] = package["uuid"].get<std::string>();
							materialJson["index"] = i;

							json["materials"].push_back(materialJson);
						}
					}
				}

				sceneJson.push_back(std::move(json));
			}
		}

		writer["scene"] = sceneJson;
	}

	void
	EntitiesModule::disconnect() noexcept
	{
		this->enable.disconnect();
		this->objects.disconnect();
	}
}