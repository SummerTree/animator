#ifndef OCTOON_FBX_IMPORTER_H_
#define OCTOON_FBX_IMPORTER_H_

#include <octoon/game_object.h>
#include <octoon/material/material.h>
#include <octoon/asset_importer.h>
#include <filesystem>
#include <fbxsdk.h>

namespace octoon
{
	class OCTOON_EXPORT FBXImporter final : public AssetImporter
	{
		OctoonDeclareSubClass(FBXImporter, AssetImporter)
	public:
		FBXImporter() noexcept;
		FBXImporter(const std::filesystem::path& path) noexcept;
		~FBXImporter() noexcept;

		virtual std::shared_ptr<Object> onImportAsset() noexcept(false) override;

		static std::vector<std::filesystem::path> getDependencies(const std::filesystem::path& filepath) noexcept(false);

	private:
		FBXImporter(const FBXImporter&) = delete;
		FBXImporter& operator=(const FBXImporter&) = delete;

	private:
		GameObjectPtr ParseMesh(FbxNode* node, const std::filesystem::path& path);
		GameObjectPtr ProcessNode(FbxScene* scene, FbxNode* node, const std::filesystem::path& path);

		std::size_t LoadMaterial(FbxMesh* mesh, std::vector<std::shared_ptr<Material>>& materials, const std::filesystem::path& path);
		std::shared_ptr<Material> LoadMaterialAttribute(FbxSurfaceMaterial* surfaceMaterial, const std::filesystem::path& path);
	};
}

#endif