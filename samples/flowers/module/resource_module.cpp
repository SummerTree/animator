#include "resource_module.h"
#include <shlobj.h>
#include <filesystem>

namespace flower
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
		{
			this->rootPath = std::filesystem::path(path).append(".flower").string();
			if (!std::filesystem::exists(this->rootPath))
				std::filesystem::create_directory(this->rootPath);
		}
		else
		{
			this->rootPath = "../../system";
		}

		this->hdriPath = std::filesystem::path(this->rootPath).append("hdri").string();
		this->materialPath = std::filesystem::path(this->rootPath).append("materials").string();
#else
		this->hdriPath = "../../system/hdri";
		this->materialPath = "../../system/materials";
#endif
	}

	void 
	ResourceModule::load(octoon::runtime::json& reader) noexcept
	{
	}

	void 
	ResourceModule::save(octoon::runtime::json& writer) noexcept
	{
	}
}