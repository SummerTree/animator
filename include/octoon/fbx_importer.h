#ifndef OCTOON_FBX_IMPORTER_H_
#define OCTOON_FBX_IMPORTER_H_

#include <octoon/game_object.h>
#include <octoon/asset_importer.h>
#include <filesystem>

namespace octoon
{
	class OCTOON_EXPORT FBXImporter final : public AssetImporter
	{
		OctoonDeclareSubClass(FBXImporter, AssetImporter)
	public:
		FBXImporter() noexcept;
		~FBXImporter() noexcept;

		static bool doCanRead(std::istream& stream) noexcept;
		static bool doCanRead(const char* type) noexcept;

		static std::vector<std::filesystem::path> getDependencies(const std::filesystem::path& filepath) noexcept(false);
		static std::shared_ptr<GameObject> load(const std::filesystem::path& filepath) noexcept(false);

	private:
		FBXImporter(const FBXImporter&) = delete;
		FBXImporter& operator=(const FBXImporter&) = delete;
	};
}

#endif