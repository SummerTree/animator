#ifndef OCTOON_ASS_LOADER_H_
#define OCTOON_ASS_LOADER_H_

#include <octoon/io/iostream.h>
#include <octoon/game_object.h>
#include <octoon/asset_importer.h>
#include <filesystem>

namespace octoon
{
	class OCTOON_EXPORT ASSImporter final : public AssetImporter
	{
		OctoonDeclareSubClass(ASSImporter, AssetImporter)
	public:
		ASSImporter() noexcept;
		~ASSImporter() noexcept;

		static bool doCanRead(io::istream& stream) noexcept;
		static bool doCanRead(const char* type) noexcept;

		static GameObjects load(const std::filesystem::path& filepath) noexcept(false);

	private:
		ASSImporter(const ASSImporter&) = delete;
		ASSImporter& operator=(const ASSImporter&) = delete;
	};
}

#endif