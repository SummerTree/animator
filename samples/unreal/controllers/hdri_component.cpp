#include "hdri_component.h"
#include "unreal_behaviour.h"
#include <octoon/image/image.h>
#include <octoon/runtime/uuid.h>
#include <fstream>
#include <filesystem>

namespace unreal
{
	HDRiComponent::HDRiComponent() noexcept
	{
	}

	HDRiComponent::~HDRiComponent() noexcept
	{
	}

	nlohmann::json
	HDRiComponent::importPackage(std::string_view filepath) noexcept(false)
	{
		octoon::Image image;

		if (image.load(std::string(filepath)))
		{
			auto width = image.width();
			auto height = image.height();
			auto data = (float*)image.data();
			auto size = width * height * 3;
			auto pixels = std::make_unique<std::uint8_t[]>(size);

			for (std::size_t i = 0; i < size; i += 3)
			{
				pixels[i] = std::clamp<float>(std::pow(data[i], 1 / 2.2) * 255.0f, 0, 255);
				pixels[i + 1] = std::clamp<float>(std::pow(data[i + 1], 1 / 2.2) * 255.0f, 0, 255);
				pixels[i + 2] = std::clamp<float>(std::pow(data[i + 2], 1 / 2.2) * 255.0f, 0, 255);
			}

			auto uuid = octoon::make_guid();
			auto rootPath = std::filesystem::path(this->getModel()->hdriPath).append(uuid);
			auto hdriPath = std::filesystem::path(rootPath).append(uuid + ".hdr");
			auto previewPath = std::filesystem::path(rootPath).append(uuid + ".png");
			auto packagePath = std::filesystem::path(rootPath).append("package.json");

			std::filesystem::create_directory(this->getModel()->hdriPath);
			std::filesystem::create_directory(rootPath);
			std::filesystem::copy(filepath, hdriPath);

			QImage qimage(pixels.get(), width, height, QImage::Format::Format_RGB888);
			qimage = qimage.scaled(260, 130);
			if (!qimage.save(QString::fromStdString(previewPath.string())))
				throw std::runtime_error("Cannot generate image for preview");

			nlohmann::json item;
			item["uuid"] = uuid;
			item["name"] = std::filesystem::path(filepath).filename().u8string();
			item["preview"] = previewPath.u8string();
			item["path"] = hdriPath.u8string();

			std::ofstream ifs(packagePath, std::ios_base::binary);
			if (ifs)
			{
				auto dump = item.dump();
				ifs.write(dump.c_str(), dump.size());
			}

			this->getModel()->hdriIndexList_.getValue().push_back(uuid);

			return item;
		}

		return nlohmann::json();
	}

	nlohmann::json
	HDRiComponent::getPackage(std::string_view uuid) noexcept
	{
		auto it = this->packageList_.find(std::string(uuid));
		if (it == this->packageList_.end())
		{
			std::ifstream ifs(std::filesystem::path(this->getModel()->hdriPath).append(uuid).append("package.json"));
			if (ifs)
			{
				auto package = nlohmann::json::parse(ifs);
				this->packageList_[std::string(uuid)] = package;
				return package;
			}
			else
			{
				return nlohmann::json();
			}
		}

		return this->packageList_[std::string(uuid)];
	}

	bool
	HDRiComponent::removePackage(std::string_view uuid) noexcept
	{
		try
		{
			auto& indexList_ = this->getModel()->hdriIndexList_.getValue();

			for (auto it = indexList_.begin(); it != indexList_.end(); ++it)
			{
				if ((*it).get<nlohmann::json::string_t>() == uuid)
				{
					auto packagePath = std::filesystem::path(this->getModel()->hdriPath).append(uuid);
					std::filesystem::remove_all(packagePath);

					auto package = this->packageList_.find(std::string(uuid));
					if (package != this->packageList_.end())
						this->packageList_.erase(package);

					indexList_.erase(it);
					return true;
				}
			}
		}
		catch (...)
		{
		}

		return false;
	}

	void
	HDRiComponent::save() noexcept(false)
	{
		try
		{
			if (!std::filesystem::exists(this->getModel()->hdriPath))
				std::filesystem::create_directory(this->getModel()->hdriPath);

			std::ofstream ifs(this->getModel()->hdriPath + "/index.json", std::ios_base::binary);
			if (ifs)
			{
				auto& indexList_ = this->getModel()->hdriIndexList_.getValue();
				auto data = indexList_.dump();
				ifs.write(data.c_str(), data.size());
			}
		}
		catch (...)
		{
		}
	}

	void
	HDRiComponent::initPackageIndices() noexcept(false)
	{
		auto& indexList_ = this->getModel()->hdriIndexList_.getValue();

		std::ifstream indexStream(this->getModel()->hdriPath + "/index.json");
		if (indexStream)
			indexList_ = nlohmann::json::parse(indexStream);

		bool needUpdateIndexFile = false;

		std::set<std::string> indexSet;

		for (auto& it : indexList_)
		{
			if (!std::filesystem::exists(std::filesystem::path(this->getModel()->hdriPath).append(it.get<nlohmann::json::string_t>())))
				needUpdateIndexFile = true;
			else
				indexSet.insert(it.get<nlohmann::json::string_t>());
		}

		for (auto& it : std::filesystem::directory_iterator(this->getModel()->hdriPath))
		{
			if (std::filesystem::is_directory(it))
			{
				auto filepath = it.path();
				auto filename = filepath.filename();

				auto index = indexSet.find(filename.string());
				if (index == indexSet.end())
				{
					if (std::filesystem::exists(filepath.append("package.json")))
					{
						needUpdateIndexFile = true;
						indexSet.insert(filename.string());
					}
				}
			}
		}

		if (needUpdateIndexFile)
		{
			nlohmann::json json;
			for (auto& it : indexSet)
				json += it;

			this->getModel()->hdriIndexList_.getValue() = json;
			this->save();
		}
	}

	void
	HDRiComponent::onEnable() noexcept
	{
		if (std::filesystem::exists(this->getModel()->hdriPath))
			this->initPackageIndices();
		else
			std::filesystem::create_directory(this->getModel()->hdriPath);
	}

	void
	HDRiComponent::onDisable() noexcept
	{
	}
}