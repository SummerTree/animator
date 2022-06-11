#include "resource_module.h"
#include <shlobj.h>
#include <filesystem>

namespace unreal
{
	ResourceModule::ResourceModule() noexcept
	{
		this->reset();
	}

	ResourceModule::~ResourceModule() noexcept
	{
	}

	void
	ResourceModule::reset() noexcept
	{
#ifdef _WINDOWS_
		char path[MAX_PATH];
		if (SHGetFolderPathA(NULL, CSIDL_PROFILE, NULL, 0, path) == S_OK)
			this->rootPath = std::filesystem::path(path).append(".animator").string();
		else
			this->rootPath = "../../system";

		this->cachePath = std::filesystem::path(this->rootPath).append("cache").string();
#else
		this->rootPath = "../../system";
		this->cachePath = "../../system/cache";
#endif
	}

	void 
	ResourceModule::load(nlohmann::json& reader, std::shared_ptr<octoon::AssetBundle>& ab) noexcept
	{
	}

	void 
	ResourceModule::save(nlohmann::json& writer, std::shared_ptr<octoon::AssetBundle>& ab) noexcept
	{
	}
}