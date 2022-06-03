#include "entities_module.h"
#include "../importer/model_importer.h"
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
						auto path = animationJson["path"].get<nlohmann::json::string_t>();

						if (type == 0)
						{
							auto animationData = octoon::VMDLoader::loadMotion(root + path);
							if (animationData)
							{
								auto animator = object->addComponent<octoon::AnimatorComponent>(std::move(animationData), object->getComponent<octoon::SkinnedMeshRendererComponent>()->getTransforms());
								animator->setName(animationJson["uuid"].get<nlohmann::json::string_t>());
							}
						}
						else
						{
							auto animationData = octoon::VMDLoader::loadMorph(root + path);
							if (animationData)
							{
								auto animator = object->addComponent<octoon::AnimatorComponent>(std::move(animationData));
								animator->setName(animationJson["uuid"].get<nlohmann::json::string_t>());
							}
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
						auto animationName = animation->getName();
						auto fileName = (animationName.empty() ? octoon::make_guid() : animation->getName()) + ".vmd";

						if (animation->getAvatar().empty())
						{
							octoon::VMDLoader::saveMorph(root + fileName, *animation->getAnimation());

							nlohmann::json animationJson;
							animationJson["type"] = 1;
							animationJson["uuid"] = animationName;
							animationJson["path"] = fileName;
							json["animation"].push_back(std::move(animationJson));
						}
						else
						{
							octoon::VMDLoader::saveMotion(root + fileName, *animation->getAnimation());

							nlohmann::json animationJson;
							animationJson["type"] = 0;
							animationJson["uuid"] = animationName;
							animationJson["path"] = fileName;
							json["animation"].push_back(std::move(animationJson));
						}
					}
				}

				sceneJson.push_back(std::move(json));
			}
		}

		writer["scene"] = sceneJson;
	}
}