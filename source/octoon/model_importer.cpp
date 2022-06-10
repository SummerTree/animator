#include <octoon/model_importer.h>
#include <octoon/asset_database.h>
#include <octoon/texture/texture.h>
#include <octoon/pmx_loader.h>
#include <octoon/runtime/uuid.h>
#include <octoon/runtime/string.h>
#include <octoon/mesh_animation_component.h>
#include <octoon/mesh/sphere_mesh.h>
#include <fstream>
#include <filesystem>
#include <codecvt>

namespace octoon
{
	OctoonImplementSingleton(ModelImporter)

	ModelImporter::ModelImporter() noexcept
	{
	}

	ModelImporter::~ModelImporter() noexcept
	{
		this->close();
	}

	nlohmann::json
	ModelImporter::createPackage(std::string_view filepath) noexcept(false)
	{
		octoon::PMX pmx;

		if (octoon::PMX::load(filepath, pmx))
		{
			if (pmx.description.japanModelLength == 0)
			{
				std::wstring wfilepath = std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t>{}.from_bytes(std::string(filepath));
				auto filename = std::filesystem::path(wfilepath).filename().wstring();

				pmx.description.japanCommentName.resize(filename.size() + 1);
				pmx.description.japanModelLength = pmx.description.japanCommentName.size() * 2;
				std::memcpy(pmx.description.japanCommentName.data(), filename.data(), pmx.description.japanModelLength);
			}

			auto package = AssetDatabase::instance()->createAsset(pmx, assertPath_);
			if (package.is_object())
			{
				indexList_.push_back(package["uuid"].get<std::string>());
				return package;
			}
		}

		return nlohmann::json();
	}

	nlohmann::json
	ModelImporter::createPackage(const octoon::GameObjectPtr& gameObject) const noexcept
	{
		auto it = assetList_.find(gameObject);
		if (it != assetList_.end())
		{
			auto& package = (*it).second;

			nlohmann::json json;
			json["uuid"] = package["uuid"].get<nlohmann::json::string_t>();

			return json;
		}
		auto path = assetPathList_.find(gameObject);
		if (path != assetPathList_.end())
		{
			nlohmann::json json;
			json["path"] = (char*)(*path).second.c_str();

			return json;
		}
		auto assetPath = AssetDatabase::instance()->getAssetPath(gameObject);
		if (!assetPath.empty())
		{
			nlohmann::json json;
			json["path"] = assetPath;

			return json;
		}

		return std::string();
	}

	octoon::GameObjectPtr
	ModelImporter::loadPackage(const nlohmann::json& package, octoon::PMXLoadFlags flags) noexcept(false)
	{
		if (package["path"].is_string())
		{
			auto path = package["path"].get<nlohmann::json::string_t>();
			auto model = octoon::PMXLoader::load(path, flags);
			if (model)
			{
				assetList_[model] = package;
				return model;
			}
		}

		return nullptr;
	}

	octoon::GameObjectPtr
	ModelImporter::loadMetaData(const nlohmann::json& metadata, octoon::PMXLoadFlags flags) noexcept
	{
		if (metadata.find("uuid") != metadata.end())
		{
			auto uuid = metadata["uuid"].get<nlohmann::json::string_t>();
			auto package = this->getPackage(uuid);
			if (package.is_object())
				return this->loadPackage(package, flags);
		
		}
		if (metadata.find("path") != metadata.end())
		{
			auto path = metadata["path"].get<nlohmann::json::string_t>();
			auto texture = AssetDatabase::instance()->loadAssetAtPath(path, flags);
			if (texture) return texture->downcast_pointer<octoon::GameObject>();
		}

		return nullptr;
	}
}