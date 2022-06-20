#include "unreal_profile.h"
#include "unreal_version.h"
#include <fstream>
#include <filesystem>
#include <octoon/runtime/json.h>
#include <octoon/asset_bundle.h>

namespace unreal
{
	UnrealProfile::UnrealProfile() noexcept
		: version(UNREAL_VERSION)
		, physicsModule(std::make_shared<PhysicsModule>())
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

	UnrealProfile::UnrealProfile(const std::filesystem::path& path_) noexcept(false)
		: UnrealProfile()
	{
		this->load(path_);
	}

	UnrealProfile::~UnrealProfile() noexcept
	{
		this->reset();
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
	UnrealProfile::reset() noexcept
	{
		this->path.clear();
		this->physicsModule->reset();
		this->encodeModule->reset();
		this->playerModule->reset();
		this->soundModule->reset();
		this->entitiesModule->reset();
		this->offlineModule->reset();
		this->recordModule->reset();
		this->markModule->reset();
		this->mainLightModule->reset();
		this->environmentLightModule->reset();
		this->cameraModule->reset();
		this->resourceModule->reset();
		this->selectorModule->reset();
		this->gridModule->reset();
		this->ab.reset();
	}

	void
	UnrealProfile::load(const std::filesystem::path& path_) noexcept(false)
	{
		std::ifstream stream(std::filesystem::path(path_).append("manifest.json"));
		if (stream)
		{
			auto json = nlohmann::json::parse(stream);

			this->path = path_;
			this->ab = octoon::AssetBundle::instance()->loadFromFile(std::filesystem::path(path_).append("Assets"));

			if (json.contains("version") && json["physics"].is_string())
				this->version = json["version"].get<std::string>();
			if (json.contains("physics") && json["physics"].is_object())
				this->physicsModule->load(json["physics"], this->ab);
			if (json.contains("encode") && json["encode"].is_object())
				this->encodeModule->load(json["encode"], this->ab);
			if (json.contains("time") && json["time"].is_object())
				this->playerModule->load(json["time"], this->ab);
			if (json.contains("sound") && json["sound"].is_object())
				this->soundModule->load(json["sound"], this->ab);
			if (json.contains("entities") && json["entities"].is_object())
				this->entitiesModule->load(json["entities"], this->ab);
			if (json.contains("offline") && json["offline"].is_object())
				this->offlineModule->load(json["offline"], this->ab);
			if (json.contains("canvas") && json["canvas"].is_object())
				this->recordModule->load(json["canvas"], this->ab);
			if (json.contains("camera") && json["camera"].is_object())
				this->cameraModule->load(json["camera"], this->ab);
			if (json.contains("mark") && json["mark"].is_object())
				this->markModule->load(json["mark"], this->ab);
			if (json.contains("mainLight") && json["mainLight"].is_object())
				this->mainLightModule->load(json["mainLight"], this->ab);
			if (json.contains("environmentLight") && json["environmentLight"].is_object())
				this->environmentLightModule->load(json["environmentLight"], this->ab);
			if (json.contains("resource") && json["resource"].is_object())
				this->resourceModule->load(json["resource"], this->ab);
			if (json.contains("drag") && json["drag"].is_object())
				this->selectorModule->load(json["drag"], this->ab);
			if (json.contains("grid") && json["grid"].is_object())
				this->gridModule->load(json["grid"], this->ab);

			this->ab->unload();
		}
		else
		{
			throw std::runtime_error("Can't find manifest.json in " + path_.string());
		}
	}

	void
	UnrealProfile::save(const std::filesystem::path& path_) noexcept(false)
	{
		auto backupFile = std::filesystem::path(path_).append("manifest.json!");
		auto manifestFile = std::filesystem::path(path_).append("manifest.json");

		try
		{
			if (!std::filesystem::exists(path_))
				std::filesystem::create_directories(path_);

			if (std::filesystem::exists(backupFile))
				std::filesystem::remove(backupFile);

			if (std::filesystem::exists(manifestFile))
				std::filesystem::rename(manifestFile, backupFile);

			std::ofstream stream(manifestFile);
			if (stream)
			{
				nlohmann::json json;
				json["version"] = UNREAL_VERSION;

				if (!this->ab)
					this->ab = octoon::AssetBundle::instance()->loadFromFile(std::filesystem::path(path_).append("Assets"));

				this->path = path_;
				this->physicsModule->save(json["physics"], ab);
				this->encodeModule->save(json["encode"], ab);
				this->playerModule->save(json["time"], ab);
				this->soundModule->save(json["sound"], ab);
				this->entitiesModule->save(json["entities"], ab);
				this->offlineModule->save(json["offline"], ab);
				this->cameraModule->save(json["camera"], ab);
				this->recordModule->save(json["canvas"], ab);
				this->markModule->save(json["mark"], ab);
				this->mainLightModule->save(json["mainLight"], ab);
				this->environmentLightModule->save(json["environmentLight"], ab);
				this->resourceModule->save(json["resource"], ab);
				this->selectorModule->save(json["drag"], ab);
				this->gridModule->save(json["grid"], ab);

				this->ab->saveAssets();

				auto string = json.dump();
				stream.write(string.c_str(), string.size());

				if (std::filesystem::exists(backupFile))
					std::filesystem::remove(backupFile);
			}
			else
			{
				throw std::runtime_error("Failed to create file: " + path_.string());
			}
		}
		catch (std::exception& e)
		{
			if (std::filesystem::exists(manifestFile))
				std::filesystem::remove(manifestFile);

			if (std::filesystem::exists(backupFile))
				std::filesystem::rename(backupFile, manifestFile);

			throw e;
		}
	}
}