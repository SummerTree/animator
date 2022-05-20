#include "hdri_component.h"
#include "flower_behaviour.h"
#include <octoon/image/image.h>
#include <fstream>
#include <filesystem>
#include <quuid.h>

namespace flower
{
	HDRiComponent::HDRiComponent() noexcept
	{
	}

	HDRiComponent::~HDRiComponent() noexcept
	{
	}

	nlohmann::json
	HDRiComponent::importHDRi(std::string_view filepath) noexcept(false)
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

			auto id = QUuid::createUuid().toString();
			auto uuid = id.toStdString().substr(1, id.length() - 2);

			auto rootPath = std::filesystem::path(this->getModel()->hdriPath).append(uuid);
			auto hdriPath = std::filesystem::path(rootPath).append(uuid + ".hdr");
			auto previewPath = std::filesystem::path(rootPath).append(uuid + ".png");
			auto packagePath = std::filesystem::path(rootPath).append("package.json");

			std::filesystem::create_directory(rootPath);
			std::filesystem::copy(filepath, hdriPath);

			QImage qimage(pixels.get(), width, height, QImage::Format::Format_RGB888);
			qimage = qimage.scaled(260, 130);
			if (!qimage.save(QString::fromStdString(previewPath.string())))
				throw std::runtime_error("Cannot generate image for preview");

			nlohmann::json item;
			item["uuid"] = uuid;
			item["name"] = std::filesystem::path(filepath).filename().string();
			item["preview"] = previewPath.string();
			item["path"] = hdriPath.string();

			std::ofstream ifs(packagePath, std::ios_base::binary);
			if (ifs)
			{
				auto dump = item.dump();
				ifs.write(dump.c_str(), dump.size());
			}

			indexList_.push_back(uuid);

			return item;
		}

		return nlohmann::json();
	}

	void
	HDRiComponent::save() noexcept(false)
	{
		if (!std::filesystem::exists(this->getModel()->hdriPath))
			std::filesystem::create_directory(this->getModel()->hdriPath);

		std::ofstream ifs(this->getModel()->hdriPath + "/index.json", std::ios_base::binary);
		if (ifs)
		{
			auto data = indexList_.dump();
			ifs.write(data.c_str(), data.size());
		}
	}

	const nlohmann::json&
	HDRiComponent::getIndexList() const noexcept
	{
		return this->indexList_;
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

	void
	HDRiComponent::initResourceList() noexcept
	{
		try
		{
			std::ifstream indexStream(this->getModel()->hdriPath + "/index.json");
			if (indexStream)
				this->indexList_ = nlohmann::json::parse(indexStream);

			bool needUpdateIndexFile = false;

			std::set<std::string> indexSet;

			for (auto& it : this->indexList_)
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

				this->indexList_ = json;
				this->save();
			}
		}
		catch (...)
		{
		}
	}

	void
	HDRiComponent::onEnable() noexcept
	{
		this->initResourceList();
	}

	void
	HDRiComponent::onDisable() noexcept
	{
	}
}