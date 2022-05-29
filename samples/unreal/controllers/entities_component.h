#ifndef UNREAL_ENTITIES_COMPONENT_H_
#define UNREAL_ENTITIES_COMPONENT_H_

#include "../utils/pmm.h"
#include "../unreal_component.h"
#include "../module/sound_module.h"
#include "../module/entities_module.h"

#include <optional>
#include <octoon/octoon.h>

namespace unreal
{
	class EntitiesComponent final : public UnrealComponent<EntitiesModule>
	{
	public:
		EntitiesComponent() noexcept;
		virtual ~EntitiesComponent() noexcept;

		void importAbc(std::string_view path) noexcept(false);
		void importAss(std::string_view path) noexcept(false);
		void importPMM(std::string_view path) noexcept(false);
		bool importModel(std::string_view path) noexcept;
		void importHDRi(std::string_view path) noexcept;
		void importHDRi(const std::shared_ptr<octoon::GraphicsTexture>& texture) noexcept;

		bool exportModel(std::string_view path) noexcept;

		void clearHDRi() noexcept;

		virtual const std::type_info& type_info() const noexcept
		{
			return typeid(EntitiesComponent);
		}

	private:
		octoon::GameObjectPtr createCamera(const octoon::PMMFile& pmm) noexcept;
		void setupBoneAnimation(const octoon::PmmModel& model, octoon::AnimationClips<float>& clips) noexcept;
		void setupMorphAnimation(const octoon::PmmModel& model, octoon::AnimationClip<float>& clips) noexcept;
		void setupCameraAnimation(const std::vector<octoon::PmmKeyframeCamera>& camera, octoon::Animation<float>& animtion) noexcept;

	private:
		void onInit() noexcept override;

		void onEnable() noexcept override;
		void onDisable() noexcept override;
	};
}

#endif