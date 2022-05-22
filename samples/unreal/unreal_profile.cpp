#include "unreal_profile.h"
#include <fstream>
#include <filesystem>
#include <octoon/runtime/json.h>

namespace unreal
{
	UnrealProfile::UnrealProfile() noexcept
		: physicsModule(std::make_shared<PhysicsModule>())
		, encodeModule(std::make_shared<EncodeModule>())
		, playerModule(std::make_shared<PlayerModule>())
		, fileModule(std::make_shared<FileModule>())
		, entitiesModule(std::make_shared<EntitiesModule>())
		, offlineModule(std::make_shared<OfflineModule>())
		, recordModule(std::make_shared<RecordModule>())
		, markModule(std::make_shared<MarkModule>())
		, sunModule(std::make_shared<SunModule>())
		, environmentModule(std::make_shared<EnvironmentModule>())
		, clientModule(std::make_shared<ClientModule>())
		, resourceModule(std::make_shared<ResourceModule>())
		, selectorModule(std::make_shared<SelectorModule>())
		, gridModule(std::make_shared<GridModule>())
	{
	}

	UnrealProfile::UnrealProfile(std::string_view path) noexcept(false)
		: UnrealProfile()
	{
		std::ifstream stream(path);
		if (stream)
		{
			auto json = octoon::runtime::json::parse(stream);
			this->physicsModule->load(json["physics"]);
			this->encodeModule->load(json["encode"]);
			this->playerModule->load(json["time"]);
			this->fileModule->load(json["file"]);
			this->entitiesModule->load(json["entities"]);
			this->offlineModule->load(json["offline"]);
			this->recordModule->load(json["canvas"]);
			this->markModule->load(json["mark"]);
			this->sunModule->load(json["sun"]);
			this->environmentModule->load(json["environment"]);
			this->clientModule->load(json["client"]);
			this->resourceModule->load(json["resource"]);
			this->selectorModule->load(json["drag"]);
			this->gridModule->load(json["grid"]);
		}
	}

	UnrealProfile::~UnrealProfile() noexcept
	{
	}

	std::unique_ptr<UnrealProfile>
	UnrealProfile::load(std::string_view path) noexcept(false)
	{
		return std::make_unique<UnrealProfile>(path);
	}

	void
	UnrealProfile::save(std::string_view path, const UnrealProfile& profile) noexcept(false)
	{
		std::ofstream stream(path);
		if (stream)
		{
			octoon::runtime::json json;
			profile.physicsModule->save(json["physics"]);
			profile.encodeModule->save(json["encode"]);
			profile.playerModule->save(json["time"]);
			profile.fileModule->save(json["file"]);
			profile.entitiesModule->save(json["entities"]);
			profile.offlineModule->save(json["offline"]);
			profile.recordModule->save(json["canvas"]);
			profile.markModule->save(json["mark"]);
			profile.sunModule->save(json["sun"]);
			profile.environmentModule->save(json["environment"]);
			profile.clientModule->save(json["client"]);
			profile.resourceModule->save(json["resource"]);
			profile.selectorModule->save(json["drag"]);
			profile.gridModule->save(json["grid"]);

			auto string = json.dump();
			stream.write(string.c_str(), string.size());
		}
		else
		{
			throw std::runtime_error("Failed to create file: " + std::string(path));
		}
	}
}