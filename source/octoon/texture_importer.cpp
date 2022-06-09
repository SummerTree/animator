#include <octoon/texture_importer.h>
#include <octoon/texture/texture.h>
#include <octoon/texture_loader.h>
#include <octoon/runtime/uuid.h>
#include <fstream>
#include <filesystem>
#include <set>

namespace octoon
{
	OctoonImplementSingleton(TextureImporter)

	TextureImporter::TextureImporter() noexcept
	{
	}

	TextureImporter::~TextureImporter() noexcept
	{
	}

	std::shared_ptr<octoon::GraphicsTexture>
	TextureImporter::importTexture(std::string_view path, bool generatorMipmap) noexcept(false)
	{
		auto texture = octoon::TextureLoader::load(path, generatorMipmap);
		if (texture)
		{
			assetPathList_[texture] = path;
			return texture;
		}

		return nullptr;
	}

	std::shared_ptr<octoon::GraphicsTexture>
	TextureImporter::loadPackage(const nlohmann::json& package, std::string_view outputPath) noexcept(false)
	{
		if (package.find("path") != package.end())
		{
			auto path = package["path"].get<nlohmann::json::string_t>();
			auto uuid = package["uuid"].get<nlohmann::json::string_t>();
			auto it = this->assetCache_.find(uuid);
			if (it != this->assetCache_.end())
				return this->assetCache_[uuid]->downcast_pointer<octoon::GraphicsTexture>();

			bool generateMipmap = false;
			if (package.find("mipmap") != package.end())
				generateMipmap = package["mipmap"].get<nlohmann::json::boolean_t>();

			auto texture = octoon::TextureLoader::load(path, generateMipmap);
			if (texture)
			{	
				packageList_[uuid] = package;
				assetCache_[uuid] = texture;
				assetList_[texture] = package;
				assetPathList_[texture] = path;

				return texture;
			}
		}

		return nullptr;
	}

	nlohmann::json
	TextureImporter::createPackage(std::string_view filepath, bool generateMipmap) noexcept(false)
	{
		octoon::Texture texture;

		if (texture.load(std::string(filepath)))
		{
			auto width = texture.width();
			auto height = texture.height();
			auto data = (float*)texture.data();
			auto size = width * height * 3;
			auto pixels = std::make_unique<std::uint8_t[]>(size);

			for (std::size_t i = 0; i < size; i += 3)
			{
				pixels[i] = std::clamp<float>(std::pow(data[i], 1.0f / 2.2f) * 255.0f, 0, 255);
				pixels[i + 1] = std::clamp<float>(std::pow(data[i + 1], 1.0f / 2.2f) * 255.0f, 0, 255);
				pixels[i + 2] = std::clamp<float>(std::pow(data[i + 2], 1.0f / 2.2f) * 255.0f, 0, 255);
			}

			auto uuid = octoon::make_guid();
			auto rootPath = std::filesystem::path(assertPath_).append(uuid);
			auto texturePath = std::filesystem::path(rootPath).append(uuid + ".hdr");
			auto previewPath = std::filesystem::path(rootPath).append(uuid + ".png");
			auto packagePath = std::filesystem::path(rootPath).append("package.json");

			std::filesystem::create_directories(rootPath);
			std::filesystem::copy(filepath, texturePath);
			std::filesystem::permissions(texturePath, std::filesystem::perms::owner_write);

			octoon::Texture preview(octoon::Format::R8G8B8SRGB, width, height, pixels.get());
			if (!preview.resize(260, 130).save(previewPath.string(), "png"))
				throw std::runtime_error("Cannot generate image for preview");

			nlohmann::json package;
			package["uuid"] = uuid;
			package["visible"] = true;
			package["name"] = (char*)std::filesystem::path(filepath).filename().u8string().c_str();
			package["preview"] = (char*)previewPath.u8string().c_str();
			package["path"] = (char*)texturePath.u8string().c_str();
			package["mipmap"] = generateMipmap;

			std::ofstream ifs(packagePath, std::ios_base::binary);
			if (ifs)
			{
				auto dump = package.dump();
				ifs.write(dump.c_str(), dump.size());
				ifs.close();
			}

			indexList_.push_back(uuid);

			return package;
		}

		return nlohmann::json();
	}

	nlohmann::json
	TextureImporter::createPackage(const std::shared_ptr<octoon::GraphicsTexture>& texture, std::string_view outputPath) noexcept(false)
	{
		if (texture)
		{
			auto it = this->assetPackageCache_.find(texture);
			if (it != this->assetPackageCache_.end())
				return this->assetPackageCache_[texture];

			auto uuid = octoon::make_guid();

			nlohmann::json package = this->getPackage(texture);
			if (package.find("uuid") != package.end())
			{
				uuid = package["uuid"].get<nlohmann::json::string_t>();
				for (auto& index : indexList_)
				{
					if (index == uuid)
						return package;
				}
			}

			auto filename = texture->getTextureDesc().getName();
			filename = filename.substr(filename.find_last_of("."));
			auto rootPath = std::filesystem::path(outputPath.empty() ? assertPath_ : outputPath).append(uuid);
			auto texturePath = std::filesystem::path(rootPath).append(uuid + filename);
			auto packagePath = std::filesystem::path(rootPath).append("package.json");

			std::filesystem::create_directories(rootPath);

			octoon::TextureLoader::save(texturePath.string(), texture);
			std::filesystem::permissions(texturePath, std::filesystem::perms::owner_write);

			package["uuid"] = uuid;
			package["visible"] = true;
			package["name"] = uuid + filename;
			package["path"] = (char*)texturePath.u8string().c_str();
			package["mipmap"] = texture->getTextureDesc().getMipNums() > 1;

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

					octoon::Texture image(octoon::Format::R8G8B8SRGB, width, height, pixels.get());
					image.resize(260, 130).save(texturePath2.string(), "png");

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

			this->assetPackageCache_[texture] = package;
			this->packageList_[std::string(uuid)] = package;

			return package;
		}

		return nlohmann::json();
	}
}