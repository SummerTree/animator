#ifndef OCTOON_MOTION_IMPORTER_H_
#define OCTOON_MOTION_IMPORTER_H_

#include <octoon/game_object.h>
#include <octoon/animation/animation.h>
#include <octoon/runtime/singleton.h>
#include <octoon/asset_importer.h>

namespace octoon
{
	class OCTOON_EXPORT MotionImporter final : public octoon::AssetImporter
	{
		OctoonDeclareSingleton(MotionImporter)
	public:
		MotionImporter() noexcept;
		~MotionImporter() noexcept;

		nlohmann::json importPackage(std::string_view path) noexcept(false);
		nlohmann::json createPackage(const std::shared_ptr<octoon::Animation>& animation, std::string_view outputPath = "") noexcept;
	};
}

#endif