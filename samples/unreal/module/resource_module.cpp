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

		this->hdriPath = std::filesystem::path(this->rootPath).append("hdri").string();
		this->materialPath = std::filesystem::path(this->rootPath).append("materials").string();
		this->modelPath = std::filesystem::path(this->rootPath).append("model").string();
		this->motionPath = std::filesystem::path(this->rootPath).append("motion").u8string();
		this->cachePath = std::filesystem::path(this->rootPath).append("cache").string();
#else
		this->hdriPath = "../../system/hdri";
		this->materialPath = "../../system/materials";
		this->modelPath = "../../system/model";
		this->motionPath = "../../system/motion";
		this->cachePath = "../../system/cache";
#endif
	}

	void 
	ResourceModule::load(octoon::runtime::json& reader, std::string_view path) noexcept
	{
	}

	void 
	ResourceModule::save(octoon::runtime::json& writer, std::string_view path) noexcept
	{
	}
}