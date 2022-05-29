#ifndef UNREAL_PROFILE_H_
#define UNREAL_PROFILE_H_

#include <memory>
#include <string>

#include "module/encode_module.h"
#include "module/physics_module.h"
#include "module/player_module.h"
#include "module/sound_module.h"
#include "module/entities_module.h"
#include "module/offline_module.h"
#include "module/record_module.h"
#include "module/mark_module.h"
#include "module/main_light_module.h"
#include "module/environment_module.h"
#include "module/camera_module.h"
#include "module/client_module.h"
#include "module/resource_module.h"
#include "module/selector_module.h"
#include "module/grid_module.h"

namespace unreal
{
	class UnrealProfile
	{
	public:
		UnrealProfile() noexcept;
		UnrealProfile(std::string_view path) noexcept(false);
		virtual ~UnrealProfile() noexcept;

		void disconnect() noexcept;

		static std::unique_ptr<UnrealProfile> load(std::string_view path) noexcept(false);
		static void save(std::string_view path, const UnrealProfile& profile) noexcept(false);

	private:
		UnrealProfile(const UnrealProfile&) = delete;
		UnrealProfile& operator=(const UnrealProfile&) = delete;

	public:
		std::shared_ptr<RecordModule> recordModule;
		std::shared_ptr<SoundModule> soundModule;
		std::shared_ptr<EntitiesModule> entitiesModule;
		std::shared_ptr<EncodeModule> encodeModule;
		std::shared_ptr<PhysicsModule> physicsModule;
		std::shared_ptr<PlayerModule> playerModule;
		std::shared_ptr<OfflineModule> offlineModule;
		std::shared_ptr<MarkModule> markModule;
		std::shared_ptr<ClientModule> clientModule;
		std::shared_ptr<MainLightModule> mainLightModule;
		std::shared_ptr<EnvironmentModule> environmentLightModule;
		std::shared_ptr<CameraModule> cameraModule;
		std::shared_ptr<ResourceModule> resourceModule;
		std::shared_ptr<SelectorModule> selectorModule;
		std::shared_ptr<GridModule> gridModule;
	};
}

#endif