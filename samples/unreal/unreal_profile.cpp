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
		, soundModule(std::make_shared<SoundModule>())
		, entitiesModule(std::make_shared<EntitiesModule>())
		, offlineModule(std::make_shared<OfflineModule>())
		, recordModule(std::make_shared<RecordModule>())
		, markModule(std::make_shared<MarkModule>())
		, mainLightModule(std::make_shared<MainLightModule>())
		, environmentLightModule(std::make_shared<EnvironmentModule>())
		, resourceModule(std::make_shared<ResourceModule>())
		, selectorModule(std::make_shared<SelectorModule>())
		, gridModule(std::make_shared<GridModule>())
		, cameraModule(std::make_shared<CameraModule>())
	{
	}

	UnrealProfile::UnrealProfile(std::string_view path_) noexcept(false)
		: UnrealProfile()
	{
		this->load(path_);
	}

	UnrealProfile::~UnrealProfile() noexcept
	{
	}

	void
	UnrealProfile::disconnect() noexcept
	{
		this->physicsModule->disconnect();
		this->encodeModule->disconnect();
		this->playerModule->disconnect();
		this->soundModule->disconnect();
		this->entitiesModule->disconnect();
		this->offlineModule->disconnect();
		this->recordModule->disconnect();
		this->markModule->disconnect();
		this->mainLightModule->disconnect();
		this->environmentLightModule->disconnect();
		this->cameraModule->disconnect();
		this->resourceModule->disconnect();
		this->selectorModule->disconnect();
		this->gridModule->disconnect();
	}

	void
	UnrealProfile::load(std::string_view path_) noexcept(false)
	{
		std::ifstream stream(path_);
		if (stream)
		{
			auto json = octoon::runtime::json::parse(stream);

			this->path = path;

			if (json["physics"].is_object())
				this->physicsModule->load(json["physics"]);
			if (json["encode"].is_object())
				this->encodeModule->load(json["encode"]);
			if (json["time"].is_object())
				this->playerModule->load(json["time"]);
			if (json["sound"].is_object())
				this->soundModule->load(json["sound"]);
			if (json["entities"].is_object())
				this->entitiesModule->load(json["entities"]);
			if (json["offline"].is_object())
				this->offlineModule->load(json["offline"]);
			if (json["canvas"].is_object())
				this->recordModule->load(json["canvas"]);
			if (json["camera"].is_object())
				this->cameraModule->load(json["camera"]);
			if (json["mark"].is_object())
				this->markModule->load(json["mark"]);
			if (json["mainLight"].is_object())
				this->mainLightModule->load(json["mainLight"]);
			if (json["environmentLight"].is_object())
				this->environmentLightModule->load(json["environmentLight"]);
			if (json["resource"].is_object())
				this->resourceModule->load(json["resource"]);
			if (json["drag"].is_object())
				this->selectorModule->load(json["drag"]);
			if (json["grid"].is_object())
				this->gridModule->load(json["grid"]);
		}
	}

	void
	UnrealProfile::save(std::string_view path_) noexcept(false)
	{
		std::ofstream stream(path_);
		if (stream)
		{
			octoon::runtime::json json;

			this->path = path_;
			this->physicsModule->save(json["physics"]);
			this->encodeModule->save(json["encode"]);
			this->playerModule->save(json["time"]);
			this->soundModule->save(json["sound"]);
			this->entitiesModule->save(json["entities"]);
			this->offlineModule->save(json["offline"]);
			this->cameraModule->save(json["camera"]);
			this->recordModule->save(json["canvas"]);
			this->markModule->save(json["mark"]);
			this->mainLightModule->save(json["mainLight"]);
			this->environmentLightModule->save(json["environmentLight"]);
			this->resourceModule->save(json["resource"]);
			this->selectorModule->save(json["drag"]);
			this->gridModule->save(json["grid"]);

			auto string = json.dump();
			stream.write(string.c_str(), string.size());
		}
		else
		{
			throw std::runtime_error("Failed to create file: " + std::string(path_));
		}
	}
}