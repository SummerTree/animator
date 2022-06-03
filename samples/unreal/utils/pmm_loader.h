#ifndef UNREAL_PMM_LOADER_H_
#define UNREAL_PMM_LOADER_H_

#include "../utils/pmm.h"
#include "../unreal_profile.h"
#include "../module/sound_module.h"
#include "../module/entities_module.h"

#include <optional>
#include <octoon/octoon.h>

namespace unreal
{
	class PMMLoader final
	{
	public:
		PMMLoader() noexcept;
		virtual ~PMMLoader() noexcept;

		static void load(UnrealProfile& profile, std::string_view path) noexcept(false);

	private:
		static void setupBoneAnimation(const octoon::PmmModel& model, octoon::AnimationClips<float>& clips) noexcept;
		static void setupMorphAnimation(const octoon::PmmModel& model, octoon::AnimationClip<float>& clips) noexcept;
		static void setupCameraAnimation(const std::vector<octoon::PmmKeyframeCamera>& camera, octoon::Animation<float>& animtion) noexcept;
	};
}

#endif