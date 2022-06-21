#ifndef OCTOON_PREFAB_UTILITY_H_
#define OCTOON_PREFAB_UTILITY_H_

#include <octoon/game_object.h>
#include <filesystem>

namespace octoon
{
	class OCTOON_EXPORT PrefabUtility final
	{
	public:
		PrefabUtility() noexcept;
		~PrefabUtility() noexcept;

		void saveAsPrefabAsset(const GameObjectPtr& gameObject, const std::filesystem::path& path);

	private:
		PrefabUtility(const PrefabUtility&) = delete;
		PrefabUtility& operator=(const PrefabUtility&) = delete;
	};
}

#endif