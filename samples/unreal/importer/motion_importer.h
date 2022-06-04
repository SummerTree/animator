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

		nlohmann::json importPackage(std::u8string_view path, bool blockSignals = false) noexcept(false);
		nlohmann::json getPackage(std::string_view uuid) noexcept;
		std::shared_ptr<octoon::Animation<float>> loadPackage(const nlohmann::json& package) noexcept(false);
		void removePackage(std::string_view uuid) noexcept(false);

		std::shared_ptr<octoon::Animation<float>> loadMetaData(const nlohmann::json& metadata) noexcept;
		std::shared_ptr<octoon::Animation<float>> loadCameraMetaData(const nlohmann::json& metadata) noexcept;
		nlohmann::json createMetadata(const std::shared_ptr<octoon::Animation<float>>& gameObject) const noexcept;

		MutableLiveData<nlohmann::json>& getIndexList() noexcept;

		void save() noexcept(false);

		virtual const std::type_info& type_info() const noexcept
		{
			return typeid(MotionImporter);
		}

	private:
		void initPackageIndices() noexcept(false);

	private:
		std::u8string assertPath_;

		MutableLiveData<nlohmann::json> indexList_;

		std::map<std::string, nlohmann::json> packageList_;
		std::map<std::weak_ptr<octoon::Animation<float>>, nlohmann::json, std::owner_less<std::weak_ptr<octoon::Animation<float>>>> motionList_;
		std::map<std::weak_ptr<octoon::Animation<float>>, std::u8string, std::owner_less<std::weak_ptr<octoon::Animation<float>>>> motionPathList_;
	};
}

#endif