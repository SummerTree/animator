#ifndef UNREAL_TEXTURE_IMPORTER_H_
#define UNREAL_TEXTURE_IMPORTER_H_

#include <unreal_component.h>
#include <octoon/game_object.h>
#include <octoon/hal/graphics_texture.h>
#include <octoon/runtime/singleton.h>

#include "../unreal_component.h"
#include "../module/resource_module.h"

namespace unreal
{
	class TextureImporter final
	{
		OctoonDeclareSingleton(TextureImporter)
	public:
		TextureImporter() noexcept;
		~TextureImporter() noexcept;

		void open(std::string indexPath) noexcept(false);
		void close() noexcept;

		std::shared_ptr<octoon::GraphicsTexture> importTexture(std::string_view path, bool generatorMipmap = false) noexcept(false);

		nlohmann::json createPackage(std::string_view path, bool blockSignals = false) noexcept(false);
		nlohmann::json getPackage(std::string_view uuid) noexcept;
		std::shared_ptr<octoon::GraphicsTexture> loadPackage(const nlohmann::json& package) noexcept(false);
		void removePackage(std::string_view uuid) noexcept(false);

		std::shared_ptr<octoon::GraphicsTexture> loadMetaData(const nlohmann::json& metadata) noexcept;
		nlohmann::json createMetadata(const std::shared_ptr<octoon::GraphicsTexture>& texture) const noexcept;

		MutableLiveData<nlohmann::json>& getIndexList() noexcept;

		void save() noexcept(false);

	private:
		void initPackageIndices() noexcept(false);

	private:
		std::string assertPath_;

		MutableLiveData<nlohmann::json> indexList_;

		std::map<std::string, nlohmann::json> packageList_;

		std::map<std::weak_ptr<octoon::GraphicsTexture>, nlohmann::json, std::owner_less<std::weak_ptr<octoon::GraphicsTexture>>> textureList_;
		std::map<std::weak_ptr<octoon::GraphicsTexture>, std::string, std::owner_less<std::weak_ptr<octoon::GraphicsTexture>>> texturePathList_;
	};
}

#endif