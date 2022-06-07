#include "texture_importer.h"
#include "unreal_behaviour.h"
#include <octoon/image/image.h>
#include <octoon/runtime/uuid.h>
#include <fstream>
#include <filesystem>
#include <qpixmap.h>
#include <qimage.h>

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
	TextureImporter::importTexture(std::string_view path, bool generatorMipmap) noexcept(false)
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
	TextureImporter::createPackage(std::string_view filepath, bool blockSignals) noexcept(false)
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
			auto texturePath = std::filesystem::path(rootPath).append(uuid + ".hdr");
			auto previewPath = std::filesystem::path(rootPath).append(uuid + ".png");
			auto packagePath = std::filesystem::path(rootPath).append("package.json");

			std::filesystem::create_directories(rootPath);
			std::filesystem::copy(filepath, texturePath);
			std::filesystem::permissions(texturePath, std::filesystem::perms::owner_write);

			QImage qimage(pixels.get(), width, height, QImage::Format::Format_RGB888);
			qimage = qimage.scaled(260, 130);
			if (!qimage.save(QString::fromStdString(previewPath.string())))
				throw std::runtime_error("Cannot generate image for preview");

			nlohmann::json package;
			package["uuid"] = uuid;
			package["visible"] = true;
			package["name"] = (char*)std::filesystem::path(filepath).filename().u8string().c_str();
			package["preview"] = (char*)previewPath.u8string().c_str();
			package["path"] = (char*)texturePath.u8string().c_str();

			std::ofstream ifs(packagePath, std::ios_base::binary);
			if (ifs)
			{
				auto dump = package.dump();
				ifs.write(dump.c_str(), dump.size());
				ifs.close();
			}

			indexList_.getValue().push_back(uuid);

			if (!blockSignals)
				indexList_.submit();

			return package;
		}

		return nlohmann::json();
	}

	nlohmann::json
	TextureImporter::createPackage(const std::shared_ptr<octoon::GraphicsTexture>& texture, std::string_view outputPath) noexcept(false)
	{
		if (texture)
		{
			auto it = this->textureList_.find(texture);
			if (it != this->textureList_.end())
				return this->textureList_[texture];

			auto uuid = octoon::make_guid();
			auto filename = texture->getTextureDesc().getName();
			filename = filename.substr(filename.find_last_of("."));
			auto rootPath = std::filesystem::path(outputPath.empty() ? assertPath_ : outputPath).append(uuid);
			auto texturePath = std::filesystem::path(rootPath).append(uuid + filename);
			auto packagePath = std::filesystem::path(rootPath).append("package.json");

			std::filesystem::create_directories(rootPath);

			octoon::TextureLoader::save(texturePath.string(), texture);
			std::filesystem::permissions(texturePath, std::filesystem::perms::owner_write);

			nlohmann::json package;
			package["uuid"] = uuid;
			package["visible"] = true;
			package["name"] = uuid + filename;
			package["path"] = (char*)texturePath.u8string().c_str();

			if (filename == ".hdr")
			{
				auto name = texture->getTextureDesc().getName();
				auto width = texture->getTextureDesc().getWidth();
				auto height = texture->getTextureDesc().getHeight();
				float* data_ = nullptr;

				if (texture->map(0, 0, width, height, 0, (void**)&data_))
				{
					auto size = width * height * 3;
					auto pixels = std::make_unique<std::uint8_t[]>(size);

					for (std::size_t i = 0; i < size; i += 3)
					{
						pixels[i] = std::clamp<float>(std::pow(data_[i], 1.0f / 2.2f) * 255.0f, 0, 255);
						pixels[i + 1] = std::clamp<float>(std::pow(data_[i + 1], 1.0f / 2.2f) * 255.0f, 0, 255);
						pixels[i + 2] = std::clamp<float>(std::pow(data_[i + 2], 1.0f / 2.2f) * 255.0f, 0, 255);
					}

					texture->unmap();

					auto uuid2 = octoon::make_guid();
					auto texturePath2 = std::filesystem::path(rootPath).append(uuid2 + ".png");

					QImage qimage(pixels.get(), width, height, QImage::Format::Format_RGB888);
					qimage = qimage.scaled(260, 130);
					qimage.save(QString::fromStdString(texturePath2.string()), "PNG");

					package["preview"] = texturePath2.string();
				}

				texture->unmap();
			}

			std::ofstream ifs(packagePath, std::ios_base::binary);
			if (ifs)
			{
				auto dump = package.dump();
				ifs.write(dump.c_str(), dump.size());
				ifs.close();
			}

			this->textureList_[texture] = package;
			this->packageList_[std::string(uuid)] = package;

			return package;
		}

		return nlohmann::json();
	}

	nlohmann::json
	TextureImporter::getPackage(std::string_view uuid, std::string_view outputPath) noexcept
	{
		auto it = this->packageList_.find(std::string(uuid));
		if (it == this->packageList_.end())
		{
			std::ifstream ifs(std::filesystem::path(outputPath.empty() ? assertPath_ : outputPath).append(uuid).append("package.json"));
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

	nlohmann::json
	TextureImporter::getPackage(const std::shared_ptr<octoon::GraphicsTexture>& texture) const noexcept(false)
	{
		if (texture)
		{
			auto it = textureList_.find(texture);
			if (it != textureList_.end())
			{
				auto& package = (*it).second;
				auto path = package["path"].get<nlohmann::json::string_t>();
				if (std::filesystem::exists(path))
					return package;
			}
		}

		return nlohmann::json();
	}

	std::shared_ptr<octoon::GraphicsTexture>
	TextureImporter::loadPackage(const nlohmann::json& package, bool generateMipmap, std::string_view outputPath) noexcept(false)
	{
		if (package.find("path") != package.end())
		{
			auto path = package["path"].get<nlohmann::json::string_t>();
			auto texture = octoon::TextureLoader::load(path, generateMipmap);
			if (texture)
			{
				auto uuid = package["uuid"].get<nlohmann::json::string_t>();
				auto it = this->packageList_.find(uuid);
				if (it == this->packageList_.end())
					this->packageList_[uuid] = package;

				textureList_[texture] = package;
				return texture;
			}
		}

		return nullptr;
	}

	void
	TextureImporter::removePackage(std::string_view uuid, std::string_view outputPath) noexcept(false)
	{
		auto& indexList = indexList_.getValue();

		for (auto index = indexList.begin(); index != indexList.end(); ++index)
		{
			if ((*index).get<nlohmann::json::string_t>() == uuid)
			{
				auto packagePath = std::filesystem::path(outputPath.empty() ? assertPath_ : outputPath).append(uuid);

				for (auto& it : std::filesystem::recursive_directory_iterator(packagePath))
					std::filesystem::permissions(it, std::filesystem::perms::owner_write);

				std::filesystem::remove_all(packagePath);

				auto package = this->packageList_.find(std::string(uuid));
				if (package != this->packageList_.end())
					this->packageList_.erase(package);

				indexList.erase(index);
				break;
			}
		}
	}

	MutableLiveData<nlohmann::json>&
	TextureImporter::getIndexList() noexcept
	{
		return indexList_;
	}

	void
	TextureImporter::save() noexcept(false)
	{
		if (!std::filesystem::exists(assertPath_))
			std::filesystem::create_directory(assertPath_);

		std::ofstream ifs(assertPath_ + "/index.json", std::ios_base::binary);
		if (ifs)
		{
			auto data = indexList_.getValue().dump();
			ifs.write(data.c_str(), data.size());
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