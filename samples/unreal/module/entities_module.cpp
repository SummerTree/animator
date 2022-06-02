#include "entities_module.h"
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
	EntitiesModule::load(octoon::runtime::json& reader, std::string_view path) noexcept(false)
	{
		auto root = std::string(path);
		root = root.substr(0, root.find_last_of('/')) + "/Assets/";

		if (reader["scene"].is_array())
		{
			for (auto& it : reader["scene"])
			{
				auto name = it["name"].get<nlohmann::json::string_t>();
				auto object = octoon::PMXLoader::load(root + name + "/"  + name + ".pmx");

				for (auto& animation : reader["animation"])
				{
					auto name = animation.get<nlohmann::json::string_t>();
					auto animationData = octoon::VMDLoader::loadMotion(root + name);
					object->addComponent<octoon::AnimatorComponent>(std::move(animationData), object->getComponent<octoon::SkinnedMeshRendererComponent>()->getTransforms());
				}

				this->objects.getValue().push_back(std::move(object));
			}
		}
	}

	void 
	EntitiesModule::save(octoon::runtime::json& writer, std::string_view path) noexcept(false)
	{
		auto root = std::string(path);
		root = root.substr(0, root.find_last_of('/')) + "/Assets/";

		nlohmann::json jsons;

		std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> cv;

		for (auto& it : this->objects.getValue())
		{
			nlohmann::json json;
			json["name"] = it->getName();

			auto rootPath = cv.from_bytes(root + it->getName() + "/");
			auto outputPath = cv.from_bytes(root + it->getName() + "/" + it->getName() + ".pmx");

			std::filesystem::create_directories(rootPath);

			octoon::PMXLoader::save(*it, outputPath);

			for (auto& component : it->getComponents())
			{
				if (component->isA<octoon::AnimatorComponent>())
				{
					auto animation = component->downcast<octoon::AnimatorComponent>();
					if (!animation->getAvatar().empty())
					{
						auto animationName = animation->getName();
						auto fileName = (animationName.empty() ? octoon::make_guid() : animation->getName()) + ".vmd";

						octoon::VMDLoader::saveMotion(root + fileName, *animation->getAnimation());

						writer["animation"].push_back(fileName);
					}
				}
			}

			jsons.push_back(std::move(json));
		}

		writer["scene"] = jsons;
	}
}