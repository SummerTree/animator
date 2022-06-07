#ifndef UNREAL_MOTION_IMPORTER_H_
#define UNREAL_MOTION_IMPORTER_H_

#include <octoon/game_object.h>
#include <octoon/animation/animation.h>
#include <octoon/runtime/singleton.h>

#include "../unreal_component.h"
#include "../module/resource_module.h"

namespace unreal
{
	class MotionImporter final
	{
		OctoonDeclareSingleton(MotionImporter)
	public:
		MotionImporter() noexcept;
		~MotionImporter() noexcept;

		void open(std::u8string_view indexPath) noexcept(false);
		void close() noexcept;

		std::shared_ptr<octoon::Animation<float>> importMotion(std::u8string_view path) noexcept(false);
		std::shared_ptr<octoon::Animation<float>> importCameraMotion(std::u8string_view path) noexcept(false);

		nlohmann::json createPackage(std::u8string_view path, bool blockSignals = false) noexcept(false);
		nlohmann::json createPackage(const std::shared_ptr<octoon::Animation<float>>& animation, std::u8string_view outputPath = u8"") noexcept;

		nlohmann::json getPackage(std::string_view uuid) noexcept;
		nlohmann::json getPackage(const std::shared_ptr<octoon::Animation<float>>& animation) const noexcept(false);

		std::shared_ptr<octoon::Animation<float>> loadPackage(const nlohmann::json& package) noexcept(false);
		void removePackage(std::string_view uuid) noexcept(false);

		nlohmann::json createMetadata(const std::shared_ptr<octoon::Animation<float>>& animation) const noexcept;

		MutableLiveData<nlohmann::json>& getIndexList() noexcept;

		void save() noexcept(false);

		void clearCache() noexcept;

		virtual const std::type_info& type_info() const noexcept
		{
			return typeid(MotionImporter);
		}

	private:
		void initPackageIndices() noexcept(false);

	private:
		std::u8string assertPath_;

		MutableLiveData<nlohmann::json> indexList_;

		std::map<std::string, std::shared_ptr<octoon::Animation<float>>> motionCache_;
		std::map<std::weak_ptr<octoon::Animation<float>>, nlohmann::json, std::owner_less<std::weak_ptr<octoon::Animation<float>>>> motionPackageCache_;

		std::map<std::string, nlohmann::json> packageList_;
		std::map<std::weak_ptr<octoon::Animation<float>>, nlohmann::json, std::owner_less<std::weak_ptr<octoon::Animation<float>>>> motionList_;
		std::map<std::weak_ptr<octoon::Animation<float>>, std::u8string, std::owner_less<std::weak_ptr<octoon::Animation<float>>>> motionPathList_;
	};
}

#endif