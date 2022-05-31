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

			this->path = path_;

			if (json["physics"].is_object())
				this->physicsModule->load(json["physics"], path);
			if (json["encode"].is_object())
				this->encodeModule->load(json["encode"], path);
			if (json["time"].is_object())
				this->playerModule->load(json["time"], path);
			if (json["sound"].is_object())
				this->soundModule->load(json["sound"], path);
			if (json["entities"].is_object())
				this->entitiesModule->load(json["entities"], path);
			if (json["offline"].is_object())
				this->offlineModule->load(json["offline"], path);
			if (json["canvas"].is_object())
				this->recordModule->load(json["canvas"], path);
			if (json["camera"].is_object())
				this->cameraModule->load(json["camera"], path);
			if (json["mark"].is_object())
				this->markModule->load(json["mark"], path);
			if (json["mainLight"].is_object())
				this->mainLightModule->load(json["mainLight"], path);
			if (json["environmentLight"].is_object())
				this->environmentLightModule->load(json["environmentLight"], path);
			if (json["resource"].is_object())
				this->resourceModule->load(json["resource"], path);
			if (json["drag"].is_object())
				this->selectorModule->load(json["drag"], path);
			if (json["grid"].is_object())
				this->gridModule->load(json["grid"], path);
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
			this->physicsModule->save(json["physics"], path_);
			this->encodeModule->save(json["encode"], path_);
			this->playerModule->save(json["time"], path_);
			this->soundModule->save(json["sound"], path_);
			this->entitiesModule->save(json["entities"], path_);
			this->offlineModule->save(json["offline"], path_);
			this->cameraModule->save(json["camera"], path_);
			this->recordModule->save(json["canvas"], path_);
			this->markModule->save(json["mark"], path_);
			this->mainLightModule->save(json["mainLight"], path_);
			this->environmentLightModule->save(json["environmentLight"], path_);
			this->resourceModule->save(json["resource"], path_);
			this->selectorModule->save(json["drag"], path_);
			this->gridModule->save(json["grid"], path_);

			auto string = json.dump();
			stream.write(string.c_str(), string.size());
		}
		else
		{
			throw std::runtime_error("Failed to create file: " + std::string(path_));
		}
	}
}