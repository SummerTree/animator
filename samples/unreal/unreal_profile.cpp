#include "unreal_profile.h"
#include <fstream>
#include <filesystem>
#include <octoon/runtime/json.h>
#include <octoon/asset_bundle.h>

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

	UnrealProfile::UnrealProfile(const std::filesystem::path& path_) noexcept(false)
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
	UnrealProfile::load(const std::filesystem::path& path_) noexcept(false)
	{
		std::ifstream stream(std::filesystem::path(path_).append("manifest.json"));
		if (stream)
		{
			auto json = nlohmann::json::parse(stream);
			auto ab = octoon::AssetBundle::instance()->loadFromFile(std::filesystem::path(path_).append("Assets"));

			this->path = path_.string();

			if (json["physics"].is_object())
				this->physicsModule->load(json["physics"], ab);
			if (json["encode"].is_object())
				this->encodeModule->load(json["encode"], ab);
			if (json["time"].is_object())
				this->playerModule->load(json["time"], ab);
			if (json["sound"].is_object())
				this->soundModule->load(json["sound"], ab);
			if (json["entities"].is_object())
				this->entitiesModule->load(json["entities"], ab);
			if (json["offline"].is_object())
				this->offlineModule->load(json["offline"], ab);
			if (json["canvas"].is_object())
				this->recordModule->load(json["canvas"], ab);
			if (json["camera"].is_object())
				this->cameraModule->load(json["camera"], ab);
			if (json["mark"].is_object())
				this->markModule->load(json["mark"], ab);
			if (json["mainLight"].is_object())
				this->mainLightModule->load(json["mainLight"], ab);
			if (json["environmentLight"].is_object())
				this->environmentLightModule->load(json["environmentLight"], ab);
			if (json["resource"].is_object())
				this->resourceModule->load(json["resource"], ab);
			if (json["drag"].is_object())
				this->selectorModule->load(json["drag"], ab);
			if (json["grid"].is_object())
				this->gridModule->load(json["grid"], ab);

			ab->unload();
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
			{
				std::filesystem::rename(manifestFile, backupFile);
#ifdef _WINDOWS_
				SetFileAttributesW(backupFile.c_str(), FILE_ATTRIBUTE_HIDDEN);
#endif
			}

			std::ofstream stream(manifestFile);
			if (stream)
			{
				nlohmann::json json;

				auto ab = std::make_shared<octoon::AssetBundle>();
				ab->open(std::filesystem::path(path_).append("Assets"));

				this->path = path_.string();
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

				ab->saveAssets();

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
			{
				std::filesystem::rename(backupFile, manifestFile);
#ifdef _WINDOWS_
				SetFileAttributesW(manifestFile.c_str(), FILE_ATTRIBUTE_NORMAL);
#endif
			}

			throw e;
		}
	}
}