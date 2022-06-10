#include <octoon/asset_database.h>
#include <octoon/runtime/uuid.h>
#include <octoon/vmd_loader.h>
#include <octoon/pmx_loader.h>
#include <octoon/ass_loader.h>
#include <octoon/texture/texture.h>
#include <octoon/animation/animation.h>
#include <octoon/mesh_animation_component.h>
#include <fstream>
#include <filesystem>
#include <codecvt>

namespace octoon
{
	OctoonImplementSingleton(AssetDatabase)

	AssetDatabase::AssetDatabase() noexcept
	{
	}

	AssetDatabase::~AssetDatabase() noexcept
	{
	}

	nlohmann::json
	AssetDatabase::createAsset(std::string_view filepath, std::string_view path) noexcept(false)
	{
		std::wstring u16_conv = std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t>{}.from_bytes((char*)std::string(filepath).data());

		if (std::filesystem::exists(u16_conv))
		{
			auto uuid = octoon::make_guid();
			auto extension = filepath.substr(filepath.find_last_of("."));
			auto rootPath = std::filesystem::path(path).append(uuid);
			auto motionPath = std::filesystem::path(rootPath).append(uuid + std::string(extension));
			auto packagePath = std::filesystem::path(rootPath).append("package.json");

			std::filesystem::create_directory(path);
			std::filesystem::create_directory(rootPath);
			std::filesystem::copy(u16_conv, motionPath);
			std::filesystem::permissions(motionPath, std::filesystem::perms::owner_write);

			auto filename = std::filesystem::path(u16_conv).filename().u8string();

			nlohmann::json package;
			package["uuid"] = uuid;
			package["visible"] = true;
			package["name"] = (char*)filename.substr(0, filename.find_last_of('.')).c_str();
			package["path"] = (char*)motionPath.u8string().c_str();

			std::ofstream ifs(packagePath, std::ios_base::binary);
			if (ifs)
			{
				auto dump = package.dump();
				ifs.write(dump.c_str(), dump.size());
				ifs.close();
			}

			return package;
		}

		return nlohmann::json();
	}

	nlohmann::json
	AssetDatabase::createAsset(const octoon::Texture& texture, std::string_view path) noexcept(false)
	{
		assert(!path.empty());

		auto uuid = make_guid();
		auto filename = texture.getName().substr(texture.getName().find_last_of("."));
		auto rootPath = std::filesystem::path(path).append(uuid);
		auto texturePath = std::filesystem::path(rootPath).append(uuid + filename);
		auto packagePath = std::filesystem::path(rootPath).append("package.json");

		std::filesystem::create_directories(rootPath);

		auto outputPath = texturePath.string();
		auto extension = outputPath.substr(outputPath.find_last_of(".") + 1);
		texture.save(outputPath, extension.c_str());
		std::filesystem::permissions(texturePath, std::filesystem::perms::owner_write);

		nlohmann::json package;
		package["uuid"] = uuid;
		package["visible"] = true;
		package["name"] = (char*)std::filesystem::path(texture.getName()).filename().u8string().c_str();
		package["path"] = (char*)texturePath.u8string().c_str();
		package["mipmap"] = texture.getMipLevel() > 1;

		if (filename == ".hdr")
		{
			auto name = texture.getName();
			auto width = texture.width();
			auto height = texture.height();
			float* data_ = (float*)texture.data();

			auto size = width * height * 3;
			auto pixels = std::make_unique<std::uint8_t[]>(size);

			for (std::size_t i = 0; i < size; i += 3)
			{
				pixels[i] = std::clamp<float>(std::pow(data_[i], 1.0f / 2.2f) * 255.0f, 0, 255);
				pixels[i + 1] = std::clamp<float>(std::pow(data_[i + 1], 1.0f / 2.2f) * 255.0f, 0, 255);
				pixels[i + 2] = std::clamp<float>(std::pow(data_[i + 2], 1.0f / 2.2f) * 255.0f, 0, 255);
			}

			auto uuid2 = octoon::make_guid();
			auto texturePath2 = std::filesystem::path(rootPath).append(uuid2 + ".png");

			octoon::Texture image(octoon::Format::R8G8B8SRGB, width, height, pixels.get());
			image.resize(260, 130).save(texturePath2.string(), "png");

			package["preview"] = texturePath2.string();
		}

		std::ofstream ifs(packagePath, std::ios_base::binary);
		if (ifs)
		{
			auto dump = package.dump();
			ifs.write(dump.c_str(), dump.size());
			ifs.close();
		}

		this->packageList_[std::string(uuid)] = package;

		return package;
	}

	nlohmann::json
	AssetDatabase::createAsset(const octoon::Animation& animation, std::string_view path) noexcept(false)
	{
		assert(!path.empty());

		auto uuid = make_guid();
		auto rootPath = std::filesystem::path(path).append(uuid);
		auto motionPath = std::filesystem::path(rootPath).append(uuid + ".vmd");
		auto packagePath = std::filesystem::path(rootPath).append("package.json");

		std::filesystem::create_directories(rootPath);

		octoon::VMDLoader::save(motionPath.string(), animation);
		std::filesystem::permissions(motionPath, std::filesystem::perms::owner_write);

		nlohmann::json package;
		package["uuid"] = uuid;
		package["visible"] = true;
		package["name"] = uuid + ".vmd";
		package["path"] = (char*)motionPath.u8string().c_str();

		std::ofstream ifs(packagePath, std::ios_base::binary);
		if (ifs)
		{
			auto dump = package.dump();
			ifs.write(dump.c_str(), dump.size());
			ifs.close();
		}

		this->packageList_[std::string(uuid)] = package;

		return package;
	}

	std::string
	AssetDatabase::getAssetPath(const std::shared_ptr<RttiObject>& asset) const noexcept
	{
		if (assetPathList_.contains(asset))
			return assetPathList_.at(asset);
		return std::string();
	}

	std::string
	AssetDatabase::getAssetGuid(const std::shared_ptr<RttiObject>& asset) noexcept
	{
		if (assetGuidList_.contains(asset))
			return assetGuidList_.at(asset);
		else
		{
			auto guid = octoon::make_guid();
			assetGuidList_[asset] = guid;
			return guid;
		}
	}

	std::string
	AssetDatabase::getAssetGuid(const std::shared_ptr<RttiObject>& asset) const noexcept
	{
		if (assetGuidList_.contains(asset))
			return assetGuidList_.at(asset);
		return std::string();
	}

	nlohmann::json
	AssetDatabase::getPackage(std::string_view uuid, std::string_view outputPath) noexcept
	{
		assert(!outputPath.empty());

		auto it = this->packageList_.find(std::string(uuid));
		if (it == this->packageList_.end())
		{
			std::ifstream ifs(std::filesystem::path(outputPath).append(uuid).append("package.json"));
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
	AssetDatabase::getPackage(const std::shared_ptr<RttiObject>& object) const noexcept(false)
	{
		if (object)
		{
			auto it = assetList_.find(object);
			if (it != assetList_.end())
			{
				auto& package = (*it).second;

				if (package.contains("path"))
				{
					auto path = package["path"].get<nlohmann::json::string_t>();
					if (std::filesystem::exists(path))
						return package;
				}
				else
				{
					return package;
				}				
			}
		}

		return nlohmann::json();
	}

	std::shared_ptr<RttiObject>
	AssetDatabase::loadAssetAtPath(std::string_view path) noexcept(false)
	{
		auto ext = std::string(path.substr(path.find_last_of('.')));
		for (auto& it : ext)
			it = (char)std::tolower(it);

		if (ext == ".vmd")
		{
			auto motion = octoon::VMDLoader::load(path);
			if (motion)
			{
				assetPathList_[motion] = path;
				assetGuidList_[motion] = make_guid();
				return motion;
			}
		}
		else if (ext == ".hdr" || ext == ".bmp" || ext == ".tga" || ext == ".jpg" || ext == ".png" || ext == ".jpeg" || ext == ".dds")
		{
			auto texture = std::make_shared<octoon::Texture>((std::string)path);
			if (texture)
			{
				assetPathList_[texture] = path;
				assetGuidList_[texture] = make_guid();
				return texture;
			}
		}
		else if (ext == ".pmx")
		{
			auto model = PMXLoader::load(path, PMXLoadFlagBits::AllBit);
			if (model)
			{
				assetPathList_[model] = path;
				assetGuidList_[model] = make_guid();
				return model;
			}
		}
		else if (ext == ".abc")
		{
			auto model = std::make_shared<GameObject>();
			if (model)
			{
				model->addComponent<MeshAnimationComponent>(path);
				assetPathList_[model] = path;
				assetGuidList_[model] = make_guid();
				return model;
			}
		}

		return nullptr;
	}

	std::shared_ptr<RttiObject>
	AssetDatabase::loadAssetAtPackage(const nlohmann::json& package, const Rtti& type) noexcept(false)
	{
		if (type.isDerivedFrom(Texture::getRtti()))
		{
			if (package.find("path") != package.end())
			{
				auto path = package["path"].get<nlohmann::json::string_t>();
				auto uuid = package["uuid"].get<nlohmann::json::string_t>();
				auto it = this->assetCache_.find(uuid);
				if (it != this->assetCache_.end())
					return this->assetCache_[uuid]->downcast_pointer<octoon::Texture>();

				bool generateMipmap = false;
				if (package.find("mipmap") != package.end())
					generateMipmap = package["mipmap"].get<nlohmann::json::boolean_t>();

				auto texture = std::make_shared<octoon::Texture>(path);
				if (texture)
				{
					if (generateMipmap)
						texture->setMipLevel(8);

					texture->apply();

					packageList_[uuid] = package;
					assetCache_[uuid] = texture;
					assetList_[texture] = package;
					assetPathList_[texture] = path;
					assetGuidList_[texture] = uuid;

					return texture;
				}
			}
		}
		else if (type.isDerivedFrom(Animation::getRtti()))
		{
			if (package["path"].is_string())
			{
				auto uuid = package["uuid"].get<nlohmann::json::string_t>();
				auto it = this->assetCache_.find(uuid);
				if (it != this->assetCache_.end())
					return this->assetCache_[uuid]->downcast_pointer<octoon::Animation>();

				auto path = package["path"].get<nlohmann::json::string_t>();
				auto motion = octoon::VMDLoader::load(path.c_str());
				if (motion)
				{
					packageList_[uuid] = package;
					assetCache_[uuid] = motion;
					assetList_[motion] = package;
					assetPathList_[motion] = path;
					assetGuidList_[motion] = uuid;

					return motion;
				}
			}
		}

		return nullptr;
	}
}