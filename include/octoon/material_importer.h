#ifndef UNREAL_MATERIAL_IMPORTER_H_
#define UNREAL_MATERIAL_IMPORTER_H_

#include <map>
#include <octoon/material/mesh_standard_material.h>
#include <octoon/runtime/singleton.h>

namespace octoon
{
	class OCTOON_EXPORT MaterialImporter final
	{
		OctoonDeclareSingleton(MaterialImporter)
	public:
		MaterialImporter() noexcept;
		virtual ~MaterialImporter() noexcept;

		bool addMaterial(const std::shared_ptr<octoon::Material>& material);
		std::shared_ptr<octoon::Material> getMaterial(std::string_view uuid) noexcept(false);

		nlohmann::json getPackage(std::string_view uuid) noexcept;

		nlohmann::json& getSceneList() noexcept;
		const nlohmann::json& getSceneList() const noexcept;

		std::string getAssetGuid(const std::shared_ptr<const octoon::Material>& asset) noexcept;
		std::string getAssetGuid(const std::shared_ptr<const octoon::Material>& asset) const noexcept;

	private:
		MaterialImporter(const MaterialImporter&) = delete;
		MaterialImporter& operator=(const MaterialImporter&) = delete;

	private:
		std::string texturePath_;
		std::string materialPath_;

		nlohmann::json sceneList_;
		std::map<std::string, nlohmann::json> packageList_;
		std::map<std::weak_ptr<const Material>, std::string, std::owner_less<std::weak_ptr<const Material>>> assetGuidList_;
		std::map<std::string, std::shared_ptr<octoon::Material>> materials_;
	};
}

#endif