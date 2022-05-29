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
		, clientModule(std::make_shared<ClientModule>())
		, resourceModule(std::make_shared<ResourceModule>())
		, selectorModule(std::make_shared<SelectorModule>())
		, gridModule(std::make_shared<GridModule>())
		, cameraModule(std::make_shared<CameraModule>())
	{
	}

	UnrealProfile::UnrealProfile(std::string_view path) noexcept(false)
		: UnrealProfile()
	{
		this->load(path);
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
		this->clientModule->disconnect();
		this->resourceModule->disconnect();
		this->selectorModule->disconnect();
		this->gridModule->disconnect();
	}

	void
	UnrealProfile::load(std::string_view path) noexcept(false)
	{
		std::ifstream stream(path);
		if (stream)
		{
			auto json = octoon::runtime::json::parse(stream);
			if (json.find("physics") != json.end())
				this->physicsModule->load(json["physics"]);
			if (json.find("encode") != json.end())
				this->encodeModule->load(json["encode"]);
			if (json.find("time") != json.end())
				this->playerModule->load(json["time"]);
			if (json.find("file") != json.end())
				this->soundModule->load(json["file"]);
			if (json.find("entities") != json.end())
				this->entitiesModule->load(json["entities"]);
			if (json.find("offline") != json.end())
				this->offlineModule->load(json["offline"]);
			if (json.find("canvas") != json.end())
				this->recordModule->load(json["canvas"]);
			if (json.find("camera") != json.end())
				this->cameraModule->load(json["camera"]);
			if (json.find("mark") != json.end())
				this->markModule->load(json["mark"]);
			if (json.find("mainLight") != json.end())
				this->mainLightModule->load(json["mainLight"]);
			if (json.find("environmentLight") != json.end())
				this->environmentLightModule->load(json["environmentLight"]);
			if (json.find("client") != json.end())
				this->clientModule->load(json["client"]);
			if (json.find("resource") != json.end())
				this->resourceModule->load(json["resource"]);
			if (json.find("drag") != json.end())
				this->selectorModule->load(json["drag"]);
			if (json.find("grid") != json.end())
				this->gridModule->load(json["grid"]);

			this->path = path;
		}
	}

	void
	UnrealProfile::save(std::string_view path) noexcept(false)
	{
		std::ofstream stream(path);
		if (stream)
		{
			octoon::runtime::json json;
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
			this->clientModule->save(json["client"]);
			this->resourceModule->save(json["resource"]);
			this->selectorModule->save(json["drag"]);
			this->gridModule->save(json["grid"]);

			auto string = json.dump();
			stream.write(string.c_str(), string.size());
		}
		else
		{
			throw std::runtime_error("Failed to create file: " + std::string(path));
		}
	}
}