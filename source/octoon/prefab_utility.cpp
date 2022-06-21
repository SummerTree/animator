#include <octoon/prefab_utility.h>
#include <octoon/asset_database.h>
#include <octoon/pmx_loader.h>
#include <octoon/transform_component.h>
#include <octoon/animator_component.h>
#include <octoon/mesh_filter_component.h>
#include <octoon/mesh_renderer_component.h>
#include <octoon/mesh_animation_component.h>

#include <fstream>

namespace octoon
{
	PrefabUtility::PrefabUtility() noexcept
	{
	}

	PrefabUtility::~PrefabUtility() noexcept
	{
	}

	void
	PrefabUtility::saveAsPrefabAsset(const GameObjectPtr& gameObject, const std::filesystem::path& path)
	{
		nlohmann::json prefab;

		/*auto assetPath = AssetDatabase::instance()->getAssetPath(gameObject);
		if (!assetPath.empty())
		{
			auto ext = assetPath.extension().u8string();
			for (auto& it : ext)
				it = (char)std::tolower(it);

			if (ext == u8".pmx")
			{
				auto pmx = std::make_shared<PMX>();
				if (PMX::load(assetPath, *pmx))
				{
					if (pmx->description.japanModelLength == 0)
					{
						auto filename = std::filesystem::path(assetPath).filename().wstring();

						pmx->description.japanModelName.resize(filename.size() + 1);
						pmx->description.japanModelLength = static_cast<PmxUInt32>(pmx->description.japanModelName.size() * 2);
						std::memcpy(pmx->description.japanModelName.data(), filename.data(), pmx->description.japanModelLength);
					}

					auto package = this->importAsset(pmx);

					prefab["model"]["uuid"] = package["uuid"];
				}
			}
			else
			{
				prefab["model"]["path"] = (char*)assetPath.u8string().c_str();
			}
		}

		auto transform = gameObject->getComponent<octoon::TransformComponent>();
		if (transform)
			transform->save(prefab["transform"]);

		for (auto& component : gameObject->getComponents())
		{
			if (!component->isA<AnimatorComponent>())
				continue;

			auto animation = component->downcast<AnimatorComponent>();
			if (animation->getAnimation())
			{
				auto package = this->createAsset(animation->getAnimation());
				if (package.is_object())
				{
					nlohmann::json animationJson;
					animationJson["data"] = package["uuid"].get<std::string>();
					animationJson["type"] = animation->getAvatar().empty() ? 1 : 0;

					prefab["animation"].push_back(animationJson);
				}
			}
		}

		auto meshFilter = gameObject->getComponent<MeshFilterComponent>();
		if (meshFilter)
		{
			auto mesh = meshFilter->getMesh();
			if (mesh)
			{
				auto bound = mesh->getBoundingBoxAll();
				prefab["meshFilter"]["bound"][0] = bound.box().min.to_array();
				prefab["meshFilter"]["bound"][1] = bound.box().max.to_array();
			}
		}

		auto meshRenderer = gameObject->getComponent<MeshRendererComponent>();
		if (meshRenderer)
		{
			auto& materials = meshRenderer->getMaterials();

			for (std::size_t i = 0; i < materials.size(); i++)
			{
				auto package = this->createAsset(materials[i]);
				if (package.is_object())
				{
					nlohmann::json materialJson;
					materialJson["data"] = package["uuid"].get<std::string>();
					materialJson["index"] = i;

					prefab["meshRenderer"]["materials"].push_back(materialJson);
				}
			}
		}

		auto abc = gameObject->getComponent<MeshAnimationComponent>();
		if (abc)
		{
			prefab["alembic"]["path"] = (char*)assetPath.u8string().c_str();

			for (auto& pair : abc->getMaterials())
			{
				auto materialPackage = this->createAsset(pair.second);
				if (materialPackage.is_object())
				{
					nlohmann::json materialJson;
					materialJson["data"] = materialPackage["uuid"];
					materialJson["name"] = pair.first;

					prefab["alembic"]["materials"].push_back(materialJson);
				}
			}
		}

		std::ofstream ifs(path, std::ios_base::binary);
		if (ifs)
		{
			auto dump = prefab.dump();
			ifs.write(dump.c_str(), dump.size());
		}*/
	}
}