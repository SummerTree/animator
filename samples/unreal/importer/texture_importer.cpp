#include "texture_importer.h"
#include "unreal_behaviour.h"
#include <octoon/image/image.h>
#include <octoon/runtime/uuid.h>
#include <fstream>
#include <filesystem>
#include <qpixmap.h>

namespace unreal
{
	OctoonImplementSingleton(TextureImporter)

	TextureImporter::TextureImporter() noexcept
	{
	}

	TextureImporter::~TextureImporter() noexcept
	{
	}

	void
	TextureImporter::open(std::string indexPath) noexcept(false)
	{
		if (std::filesystem::exists(indexPath))
		{
			this->assertPath_ = indexPath;
			this->initPackageIndices();
		}
	}

	void
	TextureImporter::close() noexcept
	{
	}

	std::shared_ptr<octoon::GraphicsTexture>
	TextureImporter::importHDRi(std::string_view path, bool generatorMipmap) noexcept(false)
	{
		auto ext = path.substr(path.find_last_of("."));
		if (ext == ".hdr")
		{
			auto hdri = octoon::TextureLoader::load(path, generatorMipmap);
			if (hdri)
			{
				texturePathList_[hdri] = path;
				return hdri;
			}
		}

		return nullptr;
	}

	nlohmann::json
	TextureImporter::importPackage(std::string_view filepath, bool blockSignals) noexcept(false)
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
			auto rootPath = std::filesystem::path(assertPath_).append(uuid);
			auto hdriPath = std::filesystem::path(rootPath).append(uuid + ".hdr");
			auto previewPath = std::filesystem::path(rootPath).append(uuid + ".png");
			auto packagePath = std::filesystem::path(rootPath).append("package.json");

			std::filesystem::create_directory(assertPath_);
			std::filesystem::create_directory(rootPath);
			std::filesystem::copy(filepath, hdriPath);
			std::filesystem::permissions(hdriPath, std::filesystem::perms::owner_write);

			QImage qimage(pixels.get(), width, height, QImage::Format::Format_RGB888);
			qimage = qimage.scaled(260, 130);
			if (!qimage.save(QString::fromStdString(previewPath.string())))
				throw std::runtime_error("Cannot generate image for preview");

			nlohmann::json item;
			item["uuid"] = uuid;
			item["name"] = (char*)std::filesystem::path(filepath).filename().u8string().c_str();
			item["preview"] = (char*)previewPath.u8string().c_str();
			item["path"] = (char*)hdriPath.u8string().c_str();

			std::ofstream ifs(packagePath, std::ios_base::binary);
			if (ifs)
			{
				auto dump = item.dump();
				ifs.write(dump.c_str(), dump.size());
				ifs.close();
			}

			indexList_.getValue().push_back(uuid);

			if (!blockSignals)
				indexList_.submit();

			return item;
		}

		return nlohmann::json();
	}

	nlohmann::json
	TextureImporter::getPackage(std::string_view uuid) noexcept
	{
		auto it = this->packageList_.find(std::string(uuid));
		if (it == this->packageList_.end())
		{
			std::ifstream ifs(std::filesystem::path(assertPath_).append(uuid).append("package.json"));
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
	TextureImporter::removePackage(std::string_view uuid) noexcept
	{
		try
		{
			auto& indexList = indexList_.getValue();

			for (auto it = indexList.begin(); it != indexList.end(); ++it)
			{
				if ((*it).get<nlohmann::json::string_t>() == uuid)
				{
					auto packagePath = std::filesystem::path(assertPath_).append(uuid);
					std::filesystem::remove_all(packagePath);

					auto package = this->packageList_.find(std::string(uuid));
					if (package != this->packageList_.end())
						this->packageList_.erase(package);

					indexList.erase(it);
					return true;
				}
			}
		}
		catch (...)
		{
		}

		return false;
	}

	std::shared_ptr<octoon::GraphicsTexture>
	TextureImporter::loadPackage(const nlohmann::json& package) noexcept(false)
	{
		if (package["path"].is_string())
		{
			auto path = package["path"].get<nlohmann::json::string_t>();
			auto hdri = octoon::TextureLoader::load(path, true);
			if (hdri)
			{
				textureList_[hdri] = package;
				return hdri;
			}
		}

		return nullptr;
	}

	MutableLiveData<nlohmann::json>&
	TextureImporter::getIndexList() noexcept
	{
		return indexList_;
	}

	std::shared_ptr<octoon::GraphicsTexture>
	TextureImporter::loadMetaData(const nlohmann::json& metadata) noexcept
	{
		if (metadata.find("uuid") != metadata.end())
		{
			auto uuid = metadata["uuid"].get<nlohmann::json::string_t>();
			auto package = this->getPackage(uuid);
			if (package.is_object())
				return this->loadPackage(package);
		
		}
		if (metadata.find("path") != metadata.end())
		{
			auto path = metadata["path"].get<nlohmann::json::string_t>();
			return this->importHDRi(path);
		}

		return nullptr;
	}

	nlohmann::json
	TextureImporter::createMetadata(const std::shared_ptr<octoon::GraphicsTexture>& texture) const noexcept
	{
		auto it = textureList_.find(texture);
		if (it != textureList_.end())
		{
			auto& package = (*it).second;

			nlohmann::json json;
			json["name"] = package["name"];
			json["uuid"] = package["uuid"].get<nlohmann::json::string_t>();

			return json;
		}
		auto path = texturePathList_.find(texture);
		if (path != texturePathList_.end())
		{
			nlohmann::json json;
			json["path"] = (*path).second;
			json["name"] = std::filesystem::path((*path).second).filename().string();

			return json;
		}

		return nlohmann::json();
	}

	void
	TextureImporter::save() noexcept(false)
	{
		try
		{
			if (!std::filesystem::exists(assertPath_))
				std::filesystem::create_directory(assertPath_);

			std::ofstream ifs(assertPath_ + "/index.json", std::ios_base::binary);
			if (ifs)
			{
				auto& indexList = indexList_.getValue();
				auto data = indexList.dump();
				ifs.write(data.c_str(), data.size());
			}
		}
		catch (...)
		{
		}
	}

	void
	TextureImporter::initPackageIndices() noexcept(false)
	{
		auto& indexList = indexList_.getValue();

		std::ifstream indexStream(assertPath_ + "/index.json");
		if (indexStream)
			indexList = nlohmann::json::parse(indexStream);

		bool needUpdateIndexFile = false;

		std::set<std::string> indexSet;

		for (auto& it : indexList)
		{
			if (!std::filesystem::exists(std::filesystem::path(assertPath_).append(it.get<nlohmann::json::string_t>())))
				needUpdateIndexFile = true;
			else
				indexSet.insert(it.get<nlohmann::json::string_t>());
		}

		for (auto& it : std::filesystem::directory_iterator(assertPath_))
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

			indexList_.getValue() = json;
			this->save();
		}
	}
}