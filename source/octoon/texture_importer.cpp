#include <octoon/texture_importer.h>
#include <octoon/texture/texture.h>
#include <octoon/asset_database.h>
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

	nlohmann::json
	TextureImporter::importPackage(std::string_view filepath, bool generateMipmap) noexcept(false)
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
	TextureImporter::createPackage(const std::shared_ptr<octoon::Texture>& texture, std::string_view outputPath) noexcept(false)
	{
		if (texture)
		{
			auto it = this->assetPackageCache_.find(texture);
			if (it != this->assetPackageCache_.end())
				return this->assetPackageCache_[texture];

			auto uuid = AssetDatabase::instance()->getAssetGuid(texture);

			nlohmann::json package = AssetDatabase::instance()->getPackage(texture);
			if (package.find("uuid") != package.end())
			{
				for (auto& index : indexList_)
				{
					if (index == uuid)
						return package;
				}
			}

			package = AssetDatabase::instance()->createAsset(texture, outputPath);
			assetPackageCache_[texture] = package;

			return package;
		}

		return nlohmann::json();
	}
}