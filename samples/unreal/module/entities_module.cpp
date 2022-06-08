#include "entities_module.h"
#include "../importer/model_importer.h"
#include "../importer/motion_importer.h"
#include "../importer/material_importer.h"
#include <octoon/pmx_loader.h>
#include <octoon/vmd_loader.h>
#include <octoon/animator_component.h>
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
	EntitiesModule::load(nlohmann::json& reader, std::string_view profilePath) noexcept(false)
	{
		auto root = std::string(profilePath);
		root = root.substr(0, root.find_last_of('/')) + "/Assets/";

		octoon::GameObjects objects_;

		if (reader["scene"].is_array())
		{
			for (auto& it : reader["scene"])
			{
				octoon::PMXLoadFlags flags = octoon::PMXLoadFlagBits::AllBit;
				if (it.find("materials") != it.end())
					flags = flags & ~octoon::PMXLoadFlagBits::MaterialBit;

				auto object = ModelImporter::instance()->loadMetaData(it["model"], flags);
				if (object)
				{
					if (it.contains("transform"))
					{
						auto transform = object->getComponent<octoon::TransformComponent>();
						if (transform)
							transform->load(it["transform"]);
					}

					if (it.contains("alembic"))
					{
						auto abc = object->addComponent<octoon::MeshAnimationComponent>();
						if (abc)
						{
							std::unordered_map<std::string, octoon::MaterialPtr> materials;

							for (auto& material : it["alembic"]["materials"])
							{
								auto data = material["data"].get<nlohmann::json::object_t>();
								auto name = material["name"].get<std::string>();
								materials[name] = MaterialImporter::instance()->loadPackage(data);
							}

							abc->setMaterials(materials);
							abc->load(it["alembic"]);
						}
					}

					for (auto& animationJson : it["animation"])
					{
						if (animationJson.find("data") == animationJson.end())
							continue;

						auto animation = MotionImporter::instance()->loadPackage(animationJson["data"]);
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

							auto data = materialJson["data"].get<nlohmann::json::object_t>();
							auto index = materialJson["index"].get<nlohmann::json::number_unsigned_t>();
							auto material = MaterialImporter::instance()->loadPackage(data);

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

		MaterialImporter::instance()->getSceneList().submit();
	}

	void 
	EntitiesModule::save(nlohmann::json& writer, std::string_view profilePath) noexcept(false)
	{
		auto root = std::string(profilePath);
		root = root.substr(0, root.find_last_of('/')) + "/Assets/";

		nlohmann::json sceneJson;

		std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> cv;

		for (auto& it : this->objects.getValue())
		{
			nlohmann::json json;
			json["model"] = ModelImporter::instance()->createPackage(it);

			if (json["model"].is_object())
			{
				auto transform = it->getComponent<octoon::TransformComponent>();
				if (transform)
					transform->save(json["transform"]);

				auto abc = it->getComponent<octoon::MeshAnimationComponent>();
				if (abc)
				{
					auto materialPath = root + "/Materials";
					auto texturePath = root + "/Textures";

					abc->save(json["alembic"]);

					for (auto& it : abc->getMaterials())
					{
						nlohmann::json materialJson;
						materialJson["data"] = MaterialImporter::instance()->createPackage(it.second, materialPath, texturePath);
						materialJson["name"] = it.first;

						json["alembic"]["materials"].push_back(materialJson);
					}
				}

				for (auto& component : it->getComponents())
				{
					if (component->isA<octoon::AnimatorComponent>())
					{
						auto animation = component->downcast<octoon::AnimatorComponent>();
						if (animation->getAnimation())
						{
							auto animationPath = root + "/Animation";

							nlohmann::json animationJson;
							animationJson["data"] = MotionImporter::instance()->createPackage(animation->getAnimation(), (char8_t*)animationPath.c_str());
							animationJson["type"] = animation->getAvatar().empty() ? 1 : 0;

							json["animation"].push_back(animationJson);
						}
					}
				}

				auto smr = it->getComponent<octoon::SkinnedMeshRendererComponent>();
				if (smr)
				{
					auto& materials = smr->getMaterials();
					auto materialPath = root + "/Materials";
					auto texturePath = root + "/Textures";

					for (std::size_t i = 0; i < materials.size(); i++)
					{
						nlohmann::json materialJson;
						materialJson["data"] = MaterialImporter::instance()->createPackage(materials[i], materialPath, texturePath);
						materialJson["index"] = i;

						json["materials"].push_back(materialJson);
					}
				}

				sceneJson.push_back(std::move(json));
			}
		}

		writer["scene"] = sceneJson;
	}
}