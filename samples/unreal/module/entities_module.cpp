#include "entities_module.h"
#include "../importer/model_importer.h"
#include "../importer/motion_importer.h"
#include "../importer/material_importer.h"
#include <octoon/pmx_loader.h>
#include <octoon/vmd_loader.h>
#include <octoon/animator_component.h>
#include <octoon/skinned_mesh_renderer_component.h>
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
	EntitiesModule::load(octoon::runtime::json& reader, std::string_view profilePath) noexcept(false)
	{
		auto root = std::string(profilePath);
		root = root.substr(0, root.find_last_of('/')) + "/Assets/";

		octoon::GameObjects objects_;

		if (reader["scene"].is_array())
		{
			for (auto& it : reader["scene"])
			{
				auto object = ModelImporter::instance()->loadMetaData(it["model"]);
				if (object)
				{
					for (auto& animationJson : it["animation"])
					{
						auto type = animationJson["type"].get<nlohmann::json::number_unsigned_t>();
						auto animation = MotionImporter::instance()->loadMetaData(animationJson);

						if (animation)
						{
							if (type == 0)
								auto animator = object->addComponent<octoon::AnimatorComponent>(std::move(animation), object->getComponent<octoon::SkinnedMeshRendererComponent>()->getTransforms());
							else
								auto animator = object->addComponent<octoon::AnimatorComponent>(std::move(animation));
						}
					}

					objects_.push_back(std::move(object));
				}
			}
		}

		this->objects = std::move(objects_);
	}

	void 
	EntitiesModule::save(octoon::runtime::json& writer, std::string_view profilePath) noexcept(false)
	{
		auto root = std::string(profilePath);
		root = root.substr(0, root.find_last_of('/')) + "/Assets/";

		nlohmann::json sceneJson;

		std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> cv;

		for (auto& it : this->objects.getValue())
		{
			nlohmann::json json;
			json["model"] = ModelImporter::instance()->createMetadata(it);

			if (json["model"].is_object())
			{
				for (auto& component : it->getComponents())
				{
					if (component->isA<octoon::AnimatorComponent>())
					{
						auto animation = component->downcast<octoon::AnimatorComponent>();
						if (!animation->getAnimation())
							continue;

						nlohmann::json animationJson = MotionImporter::instance()->createMetadata(animation->getAnimation());
						if (animationJson.is_object())
						{
							animationJson["type"] = animation->getAvatar().empty() ? 1 : 0;

							json["animation"].push_back(std::move(animationJson));
						}
						else
						{
							auto animationName = animation->getName();
							auto fileName = (animationName.empty() ? octoon::make_guid() : animation->getName()) + ".vmd";

							octoon::VMDLoader::saveMotion(root + fileName, *animation->getAnimation());

							animationJson["type"] = animation->getAvatar().empty() ? 1 : 0;
							animationJson["path"] = root + fileName;
							json["animation"].push_back(std::move(animationJson));
						}
					}
				}

				auto smr = it->getComponent<octoon::SkinnedMeshRendererComponent>();
				if (smr)
				{
					auto& materials = smr->getMaterials();
					for (auto& material : materials)
					{
					}
				}

				sceneJson.push_back(std::move(json));
			}
		}

		writer["scene"] = sceneJson;
	}
}