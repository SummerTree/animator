#include "hdri_component.h"
#include "flower_behaviour.h"
#include <octoon/image/image.h>
#include <fstream>
#include <filesystem>

namespace flower
{
	HDRiComponent::HDRiComponent() noexcept
	{
	}

	HDRiComponent::~HDRiComponent() noexcept
	{
	}

	void
	HDRiComponent::importHDRi(std::string_view filepath) noexcept
	{
		auto texture = octoon::TextureLoader::load(filepath);
	}

	void
	HDRiComponent::onEnable() noexcept
	{
	}

	void
	HDRiComponent::onDisable() noexcept
	{
		if (!std::filesystem::exists(this->getModel()->hdriPath))
			std::filesystem::create_directory(this->getModel()->hdriPath);

		std::ofstream ifs(this->getModel()->hdriPath + "/index.json", std::ios_base::binary);
		if (ifs)
		{
			nlohmann::json uuids;
			for (auto& mat : this->hdriList_)
				uuids.push_back(mat.first);

			auto data = uuids.dump();
			ifs.write(data.c_str(), data.size());
		}
	}
}