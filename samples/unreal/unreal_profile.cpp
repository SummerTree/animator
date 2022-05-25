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
		, mainLightModule(std::make_shared<MainLightModule>())
		, environmentLightModule(std::make_shared<EnvironmentModule>())
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
			if (json.find("physics") != json.end())
				this->physicsModule->load(json["physics"]);
			if (json.find("encode") != json.end())
				this->encodeModule->load(json["encode"]);
			if (json.find("time") != json.end())
				this->playerModule->load(json["time"]);
			if (json.find("file") != json.end())
				this->fileModule->load(json["file"]);
			if (json.find("entities") != json.end())
				this->entitiesModule->load(json["entities"]);
			if (json.find("offline") != json.end())
				this->offlineModule->load(json["offline"]);
			if (json.find("canvas") != json.end())
				this->recordModule->load(json["canvas"]);
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
		}
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
		this->fileModule->disconnect();
		this->entitiesModule->disconnect();
		this->offlineModule->disconnect();
		this->recordModule->disconnect();
		this->markModule->disconnect();
		this->mainLightModule->disconnect();
		this->environmentLightModule->disconnect();
		this->clientModule->disconnect();
		this->resourceModule->disconnect();
		this->selectorModule->disconnect();
		this->gridModule->disconnect();
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
			profile.mainLightModule->save(json["mainLight"]);
			profile.environmentLightModule->save(json["environmentLight"]);
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