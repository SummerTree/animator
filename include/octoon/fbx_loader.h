#ifndef OCTOON_FBX_LOADER_H_
#define OCTOON_FBX_LOADER_H_

#include <octoon/game_object.h>
#include <filesystem>

namespace octoon
{
	class OCTOON_EXPORT FBXLoader final
	{
	public:
		FBXLoader() noexcept;
		~FBXLoader() noexcept;

		static bool doCanRead(std::istream& stream) noexcept;
		static bool doCanRead(const char* type) noexcept;

		static std::vector<std::filesystem::path> getDependencies(const std::filesystem::path& filepath) noexcept(false);
		static std::shared_ptr<GameObject> load(const std::filesystem::path& filepath) noexcept(false);

	private:
		FBXLoader(const FBXLoader&) = delete;
		FBXLoader& operator=(const FBXLoader&) = delete;
	};
}

#endif